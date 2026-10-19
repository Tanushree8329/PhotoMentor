from flask import Flask, jsonify, render_template, request
import json
import os
import sqlite3
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
DB_PATH = Path(os.environ.get("PHOTOMENTOR_DB", ROOT / "data" / "photomentor.db"))
ENGINE_BASE = ROOT / "cpp_engine" / "photomentor_engine"
ENGINE = Path(str(ENGINE_BASE) + (".exe" if os.name == "nt" else ""))

app = Flask(__name__)
app.config["MAX_CONTENT_LENGTH"] = 5 * 1024 * 1024

def get_db():
    DB_PATH.parent.mkdir(parents=True, exist_ok=True)
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    return conn

def init_db():
    with get_db() as conn:
        conn.executescript("""
        CREATE TABLE IF NOT EXISTS lesson_progress (
            lesson_id TEXT PRIMARY KEY,
            completed INTEGER NOT NULL DEFAULT 0,
            updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
        );
        CREATE TABLE IF NOT EXISTS quiz_attempts (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            question_id TEXT NOT NULL,
            answer TEXT NOT NULL,
            correct INTEGER NOT NULL,
            created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
        );
        CREATE TABLE IF NOT EXISTS journal_entries (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            entry_date TEXT NOT NULL,
            subject TEXT NOT NULL DEFAULT '',
            settings TEXT NOT NULL DEFAULT '',
            technique TEXT NOT NULL DEFAULT '',
            notes TEXT NOT NULL DEFAULT '',
            created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
        );
        """)

def run_engine(*args):
    if not ENGINE.exists():
        raise RuntimeError(
            "C++ engine not found. Compile cpp_engine/main.cpp first. "
            "See README.md for the command."
        )
    result = subprocess.run(
        [str(ENGINE), *[str(a) for a in args]],
        capture_output=True, text=True, timeout=10, check=False
    )
    if result.returncode != 0:
        raise RuntimeError("C++ engine failed: " + result.stderr[:300])
    try:
        return json.loads(result.stdout)
    except json.JSONDecodeError as exc:
        raise RuntimeError("C++ engine returned invalid JSON.") from exc

@app.errorhandler(RuntimeError)
def handle_runtime_error(error):
    return jsonify({"error": str(error)}), 503

@app.route("/")
def home():
    return render_template("index.html")

@app.route("/api/lessons")
def api_lessons():
    lessons = run_engine("lessons")
    with get_db() as conn:
        rows = conn.execute("SELECT lesson_id, completed FROM lesson_progress").fetchall()
    status = {r["lesson_id"]: bool(r["completed"]) for r in rows}
    for lesson in lessons:
        lesson["completed"] = status.get(lesson["id"], False)
    return jsonify(lessons)

@app.route("/api/lessons/<lesson_id>/complete", methods=["POST"])
def complete_lesson(lesson_id):
    known = {item["id"] for item in run_engine("lessons")}
    if lesson_id not in known:
        return jsonify({"error": "Lesson not found"}), 404
    with get_db() as conn:
        conn.execute("""
            INSERT INTO lesson_progress(lesson_id, completed, updated_at)
            VALUES (?, 1, CURRENT_TIMESTAMP)
            ON CONFLICT(lesson_id) DO UPDATE SET
              completed=1, updated_at=CURRENT_TIMESTAMP
        """, (lesson_id,))
    return jsonify({"ok": True})

@app.route("/api/simulate", methods=["POST"])
def simulate():
    data = request.get_json(silent=True) or {}
    try:
        aperture = float(data.get("aperture", 4))
        shutter = float(data.get("shutter", 1/125))
        iso = float(data.get("iso", 100))
    except (TypeError, ValueError):
        return jsonify({"error": "Use numeric aperture, shutter and ISO values."}), 400
    if not (0.7 <= aperture <= 64 and 1/8000 <= shutter <= 30 and 25 <= iso <= 102400):
        return jsonify({"error": "Settings are outside the supported educational range."}), 400
    return jsonify(run_engine("simulate", aperture, shutter, iso))

@app.route("/api/advisor", methods=["POST"])
def advisor():
    data = request.get_json(silent=True) or {}
    subject = str(data.get("subject", "portrait"))
    light = str(data.get("light", "normal"))
    effect = str(data.get("effect", "natural"))
    if subject not in {"portrait", "landscape", "action", "night", "street", "macro"}:
        return jsonify({"error": "Choose a valid subject."}), 400
    if light not in {"low", "normal", "bright", "moving"}:
        return jsonify({"error": "Choose a valid lighting condition."}), 400
    if effect not in {"blur", "natural", "sharp"}:
        return jsonify({"error": "Choose a valid creative effect."}), 400
    return jsonify(run_engine("advisor", subject, light, effect))

@app.route("/api/quiz")
def quiz_questions():
    return jsonify(run_engine("questions"))

@app.route("/api/quiz/answer", methods=["POST"])
def quiz_answer():
    data = request.get_json(silent=True) or {}
    qid = str(data.get("questionId", ""))
    answer = str(data.get("answer", "")).upper()
    if answer not in {"A", "B", "C", "D"}:
        return jsonify({"error": "Choose one answer from A to D."}), 400
    result = run_engine("grade", qid, answer)
    if "error" in result:
        return jsonify(result), 404
    with get_db() as conn:
        conn.execute(
            "INSERT INTO quiz_attempts(question_id, answer, correct) VALUES (?, ?, ?)",
            (qid, answer, int(result["correct"]))
        )
    return jsonify(result)

@app.route("/api/progress")
def progress():
    with get_db() as conn:
        total_lessons = len(run_engine("lessons"))
        completed = conn.execute(
            "SELECT COUNT(*) AS n FROM lesson_progress WHERE completed=1"
        ).fetchone()["n"]
        attempts = conn.execute(
            "SELECT COUNT(*) AS n, COALESCE(SUM(correct),0) AS correct FROM quiz_attempts"
        ).fetchone()
        recent = conn.execute("""
            SELECT question_id, correct, created_at FROM quiz_attempts
            ORDER BY id DESC LIMIT 5
        """).fetchall()
    return jsonify({
        "totalLessons": total_lessons,
        "completedLessons": completed,
        "completionPercent": round(completed / total_lessons * 100) if total_lessons else 0,
        "quizAttempts": attempts["n"],
        "correctAnswers": attempts["correct"],
        "quizAccuracy": round(attempts["correct"] / attempts["n"] * 100) if attempts["n"] else 0,
        "recentAttempts": [dict(row) for row in recent]
    })

@app.route("/api/journal", methods=["GET", "POST"])
def journal():
    if request.method == "GET":
        with get_db() as conn:
            rows = conn.execute("""
                SELECT id, title, entry_date, subject, settings, technique, notes, created_at
                FROM journal_entries ORDER BY entry_date DESC, id DESC
            """).fetchall()
        return jsonify([dict(row) for row in rows])

    data = request.get_json(silent=True) or {}
    title = str(data.get("title", "")).strip()[:100]
    entry_date = str(data.get("entryDate", "")).strip()[:10]
    subject = str(data.get("subject", "")).strip()[:100]
    settings = str(data.get("settings", "")).strip()[:160]
    technique = str(data.get("technique", "")).strip()[:100]
    notes = str(data.get("notes", "")).strip()[:2000]
    if not title or not entry_date:
        return jsonify({"error": "Title and date are required."}), 400
    if len(entry_date) != 10 or entry_date[4] != "-" or entry_date[7] != "-":
        return jsonify({"error": "Use the date format YYYY-MM-DD."}), 400
    with get_db() as conn:
        cur = conn.execute("""
            INSERT INTO journal_entries(title, entry_date, subject, settings, technique, notes)
            VALUES (?, ?, ?, ?, ?, ?)
        """, (title, entry_date, subject, settings, technique, notes))
        entry_id = cur.lastrowid
    return jsonify({"ok": True, "id": entry_id}), 201

@app.route("/api/journal/<int:entry_id>", methods=["DELETE"])
def delete_journal_entry(entry_id):
    with get_db() as conn:
        cur = conn.execute("DELETE FROM journal_entries WHERE id=?", (entry_id,))
    if cur.rowcount == 0:
        return jsonify({"error": "Journal entry not found."}), 404
    return jsonify({"ok": True})

@app.route("/health")
def health():
    return jsonify({"status": "ok"})

init_db()

if __name__ == "__main__":
    app.run(debug=True, port=int(os.environ.get("PORT", "5000")))


from flask import Flask, jsonify, render_template, request
import json
import os
import sqlite3
import subprocess
from pathlib import Path

# --------------------------------------------------
# APPLICATION CONFIGURATION
# --------------------------------------------------

ROOT = Path(__file__).resolve().parent

# Vercel provides temporary writable storage at /tmp.
# Local development uses the project's data directory.
if os.environ.get("VERCEL"):
    DEFAULT_DB_PATH = Path("/tmp/photomentor.db")
else:
    DEFAULT_DB_PATH = ROOT / "data" / "photomentor.db"

DB_PATH = Path(
    os.environ.get("PHOTOMENTOR_DB", str(DEFAULT_DB_PATH))
)

ENGINE_BASE = ROOT / "cpp_engine" / "photomentor_engine"
ENGINE = Path(
    str(ENGINE_BASE) + (".exe" if os.name == "nt" else "")
)

app = Flask(
    __name__,
    template_folder=str(ROOT / "templates"),
    static_folder=str(ROOT / "static")
)

app.config["MAX_CONTENT_LENGTH"] = 5 * 1024 * 1024


# --------------------------------------------------
# DATABASE FUNCTIONS
# --------------------------------------------------

def get_db():
    """Open a SQLite connection and return rows by column name."""
    DB_PATH.parent.mkdir(parents=True, exist_ok=True)

    conn = sqlite3.connect(str(DB_PATH), timeout=10)
    conn.row_factory = sqlite3.Row
    return conn


def init_db():
    """Create the required tables if they do not exist."""
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


# --------------------------------------------------
# C++ ENGINE INTEGRATION
# --------------------------------------------------

def run_engine(*args):
    """
    Run the compiled PhotoMentor C++ engine.
    The executable must be built for the current OS.
    """

    if not ENGINE.is_file():
        raise RuntimeError(
            "PhotoMentor C++ engine is unavailable. "
            "The executable must be compiled for the deployment "
            "operating system and placed at: "
            + str(ENGINE)
        )

    try:
        result = subprocess.run(
            [str(ENGINE), *[str(arg) for arg in args]],
            capture_output=True,
            text=True,
            timeout=10,
            check=False,
            cwd=str(ROOT)
        )
    except subprocess.TimeoutExpired as exc:
        raise RuntimeError(
            "The C++ engine took too long to respond."
        ) from exc
    except OSError as exc:
        raise RuntimeError(
            "The C++ engine could not be started: " + str(exc)
        ) from exc

    if result.returncode != 0:
        message = result.stderr.strip()[:500]
        raise RuntimeError(
            "C++ engine failed: " + (message or "Unknown error.")
        )

    try:
        return json.loads(result.stdout)
    except json.JSONDecodeError as exc:
        raise RuntimeError(
            "The C++ engine returned invalid JSON."
        ) from exc


@app.errorhandler(RuntimeError)
def handle_runtime_error(error):
    return jsonify({"error": str(error)}), 503


# --------------------------------------------------
# WEBSITE ROUTE
# --------------------------------------------------

@app.route("/")
def home():
    return render_template("index.html")


# --------------------------------------------------
# PHOTOGRAPHY LESSONS
# --------------------------------------------------

@app.route("/api/lessons", methods=["GET"])
def api_lessons():
    lessons = run_engine("lessons")

    with get_db() as conn:
        rows = conn.execute(
            "SELECT lesson_id, completed FROM lesson_progress"
        ).fetchall()

    status = {
        row["lesson_id"]: bool(row["completed"])
        for row in rows
    }

    for lesson in lessons:
        lesson["completed"] = status.get(lesson["id"], False)

    return jsonify(lessons)


@app.route(
    "/api/lessons/<lesson_id>/complete",
    methods=["POST"]
)
def complete_lesson(lesson_id):
    known = {
        item["id"]
        for item in run_engine("lessons")
    }

    if lesson_id not in known:
        return jsonify({"error": "Lesson not found."}), 404

    with get_db() as conn:
        conn.execute("""
            INSERT INTO lesson_progress (
                lesson_id, completed, updated_at
            )
            VALUES (?, 1, CURRENT_TIMESTAMP)
            ON CONFLICT(lesson_id)
            DO UPDATE SET
                completed = 1,
                updated_at = CURRENT_TIMESTAMP
        """, (lesson_id,))

    return jsonify({"ok": True})


# --------------------------------------------------
# EXPOSURE SIMULATOR
# --------------------------------------------------

@app.route("/api/simulate", methods=["POST"])
def simulate():
    data = request.get_json(silent=True) or {}

    try:
        aperture = float(data.get("aperture", 4))
        shutter = float(data.get("shutter", 1 / 125))
        iso = float(data.get("iso", 100))
    except (TypeError, ValueError):
        return jsonify({
            "error": "Aperture, shutter and ISO must be numeric."
        }), 400

    if not (
        0.7 <= aperture <= 64
        and 1 / 8000 <= shutter <= 30
        and 25 <= iso <= 102400
    ):
        return jsonify({
            "error": "Settings are outside the supported range."
        }), 400

    return jsonify(
        run_engine("simulate", aperture, shutter, iso)
    )


# --------------------------------------------------
# CAMERA SETTINGS ADVISOR
# --------------------------------------------------

@app.route("/api/advisor", methods=["POST"])
def advisor():
    data = request.get_json(silent=True) or {}

    subject = str(data.get("subject", "portrait"))
    light = str(data.get("light", "normal"))
    effect = str(data.get("effect", "natural"))

    allowed_subjects = {
        "portrait", "landscape", "action",
        "night", "street", "macro"
    }

    allowed_lighting = {
        "low", "normal", "bright", "moving"
    }

    allowed_effects = {
        "blur", "natural", "sharp"
    }

    if subject not in allowed_subjects:
        return jsonify({
            "error": "Choose a valid photography subject."
        }), 400

    if light not in allowed_lighting:
        return jsonify({
            "error": "Choose a valid lighting condition."
        }), 400

    if effect not in allowed_effects:
        return jsonify({
            "error": "Choose a valid creative effect."
        }), 400

    return jsonify(
        run_engine("advisor", subject, light, effect)
    )


# --------------------------------------------------
# QUIZ
# --------------------------------------------------

@app.route("/api/quiz", methods=["GET"])
def quiz_questions():
    return jsonify(run_engine("questions"))


@app.route("/api/quiz/answer", methods=["POST"])
def quiz_answer():
    data = request.get_json(silent=True) or {}

    question_id = str(data.get("questionId", "")).strip()
    answer = str(data.get("answer", "")).strip().upper()

    if not question_id:
        return jsonify({
            "error": "A question ID is required."
        }), 400

    if answer not in {"A", "B", "C", "D"}:
        return jsonify({
            "error": "Choose an answer from A to D."
        }), 400

    result = run_engine("grade", question_id, answer)

    if isinstance(result, dict) and "error" in result:
        return jsonify(result), 404

    if not isinstance(result, dict) or "correct" not in result:
        return jsonify({
            "error": "The C++ engine returned an unexpected quiz result."
        }), 502

    with get_db() as conn:
        conn.execute("""
            INSERT INTO quiz_attempts (
                question_id, answer, correct
            )
            VALUES (?, ?, ?)
        """, (
            question_id,
            answer,
            int(bool(result["correct"]))
        ))

    return jsonify(result)


# --------------------------------------------------
# LEARNING PROGRESS
# --------------------------------------------------

@app.route("/api/progress", methods=["GET"])
def progress():
    total_lessons = len(run_engine("lessons"))

    with get_db() as conn:
        completed = conn.execute("""
            SELECT COUNT(*) AS n
            FROM lesson_progress
            WHERE completed = 1
        """).fetchone()["n"]

        attempts = conn.execute("""
            SELECT
                COUNT(*) AS n,
                COALESCE(SUM(correct), 0) AS correct
            FROM quiz_attempts
        """).fetchone()

        recent = conn.execute("""
            SELECT question_id, correct, created_at
            FROM quiz_attempts
            ORDER BY id DESC
            LIMIT 5
        """).fetchall()

    attempt_count = attempts["n"]
    correct_count = attempts["correct"]

    return jsonify({
        "totalLessons": total_lessons,
        "completedLessons": completed,
        "completionPercent": (
            round(completed / total_lessons * 100)
            if total_lessons else 0
        ),
        "quizAttempts": attempt_count,
        "correctAnswers": correct_count,
        "quizAccuracy": (
            round(correct_count / attempt_count * 100)
            if attempt_count else 0
        ),
        "recentAttempts": [
            dict(row) for row in recent
        ]
    })


# --------------------------------------------------
# PHOTOGRAPHY JOURNAL
# --------------------------------------------------

@app.route("/api/journal", methods=["GET", "POST"])
def journal():
    if request.method == "GET":
        with get_db() as conn:
            rows = conn.execute("""
                SELECT
                    id, title, entry_date, subject,
                    settings, technique, notes, created_at
                FROM journal_entries
                ORDER BY entry_date DESC, id DESC
            """).fetchall()

        return jsonify([
            dict(row) for row in rows
        ])

    data = request.get_json(silent=True) or {}

    title = str(data.get("title", "")).strip()[:100]
    entry_date = str(data.get("entryDate", "")).strip()
    subject = str(data.get("subject", "")).strip()[:100]
    settings = str(data.get("settings", "")).strip()[:160]
    technique = str(data.get("technique", "")).strip()[:100]
    notes = str(data.get("notes", "")).strip()[:2000]

    if not title or not entry_date:
        return jsonify({
            "error": "Title and date are required."
        }), 400

    if (
        len(entry_date) != 10
        or entry_date[4] != "-"
        or entry_date[7] != "-"
    ):
        return jsonify({
            "error": "Use the date format YYYY-MM-DD."
        }), 400

    with get_db() as conn:
        cursor = conn.execute("""
            INSERT INTO journal_entries (
                title, entry_date, subject,
                settings, technique, notes
            )
            VALUES (?, ?, ?, ?, ?, ?)
        """, (
            title, entry_date, subject,
            settings, technique, notes
        ))

        entry_id = cursor.lastrowid

    return jsonify({
        "ok": True,
        "id": entry_id
    }), 201


@app.route(
    "/api/journal/<int:entry_id>",
    methods=["DELETE"]
)
def delete_journal_entry(entry_id):
    with get_db() as conn:
        cursor = conn.execute(
            "DELETE FROM journal_entries WHERE id = ?",
            (entry_id,)
        )

    if cursor.rowcount == 0:
        return jsonify({
            "error": "Journal entry not found."
        }), 404

    return jsonify({"ok": True})


# --------------------------------------------------
# HEALTH CHECK
# --------------------------------------------------

@app.route("/health", methods=["GET"])
def health():
    return jsonify({
        "status": "ok",
        "application": "PhotoMentor"
    })


# --------------------------------------------------
# INITIALIZE DATABASE
# --------------------------------------------------

# Do not initialize the database at import time on Vercel.
# Initialize it when the application starts locally, or lazily
# through this function on the first database operation.
# This avoids a database write during serverless module import.

@app.before_request
def ensure_database():
    if not getattr(app, "_database_initialized", False):
        init_db()
        app._database_initialized = True


# --------------------------------------------------
# LOCAL DEVELOPMENT
# --------------------------------------------------

if __name__ == "__main__":
    init_db()
    app.run(
        host="0.0.0.0",
        debug=False,
        port=int(os.environ.get("PORT", "5000"))
    )

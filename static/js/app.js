const $ = (id) => document.getElementById(id);
let lessons = [];
let questions = [];
let currentQuestion = 0;
let quizScore = 0;
let answered = false;
let activeLesson = null;

async function api(url, options = {}) {
  const response = await fetch(url, {
    headers: {"Content-Type": "application/json"},
    ...options
  });
  const data = await response.json().catch(() => ({}));
  if (!response.ok) throw new Error(data.error || "Something went wrong.");
  return data;
}
function showToast(message) {
  const toast = $("toast");
  toast.textContent = message;
  toast.classList.add("show");
  clearTimeout(showToast.timer);
  showToast.timer = setTimeout(() => toast.classList.remove("show"), 2800);
}
function escapeHtml(value = "") {
  return String(value).replace(/[&<>"']/g, c => ({"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#39;"}[c]));
}

async function loadLessons(filter = "All") {
  try {
    lessons = await api("/api/lessons");
    $("statLessons").textContent = lessons.length;
    renderLessons(filter);
  } catch (error) {
    $("lessonGrid").innerHTML = `<p class="empty-state">${escapeHtml(error.message)}</p>`;
  }
}
function renderLessons(filter = "All") {
  const shown = filter === "All" ? lessons : lessons.filter(l => l.category === filter);
  $("lessonGrid").innerHTML = shown.map(lesson => `
    <article class="lesson-card">
      <div class="lesson-card-top"></div>
      <div class="lesson-card-body">
        <div class="card-meta"><span>${escapeHtml(lesson.category)}</span>${lesson.completed ? '<span class="done-tag">✓ COMPLETE</span>' : `<span>LESSON</span>`}</div>
        <h3>${escapeHtml(lesson.title)}</h3>
        <p>${escapeHtml(lesson.summary)}</p>
        <button class="card-link" data-lesson="${escapeHtml(lesson.id)}"><span>Open lesson</span><span>↗</span></button>
      </div>
    </article>`).join("") || '<p class="empty-state">No lessons in this category yet.</p>';
  $("lessonGrid").querySelectorAll("[data-lesson]").forEach(btn => {
    btn.addEventListener("click", () => openLesson(btn.dataset.lesson));
  });
}
function openLesson(id) {
  activeLesson = lessons.find(l => l.id === id);
  if (!activeLesson) return;
  $("lessonCategory").textContent = activeLesson.category.toUpperCase();
  $("lessonTitle").textContent = activeLesson.title;
  $("lessonSummary").textContent = activeLesson.summary;
  $("lessonContent").textContent = activeLesson.content;
  $("lessonExercise").textContent = activeLesson.exercise;
  $("completeLesson").textContent = activeLesson.completed ? "Completed ✓" : "Mark lesson complete ✓";
  $("completeLesson").disabled = activeLesson.completed;
  $("lessonDialog").showModal();
}

async function updateProgress() {
  try {
    const p = await api("/api/progress");
    $("statProgress").textContent = `${p.completionPercent}%`;
    $("completedLessons").innerHTML = `${p.completedLessons} <small>/ ${p.totalLessons}</small>`;
    $("completionPercent").textContent = `${p.completionPercent}%`;
    $("quizAccuracy").textContent = `${p.quizAccuracy}%`;
    $("lessonTrack").style.width = `${p.totalLessons ? p.completedLessons / p.totalLessons * 100 : 0}%`;
    $("percentTrack").style.width = `${p.completionPercent}%`;
    $("accuracyTrack").style.width = `${p.quizAccuracy}%`;
  } catch (error) { console.error(error); }
}

function formatShutter(seconds) {
  if (seconds >= 1) return `${seconds.toFixed(1).replace(/\.0$/, "")} s`;
  return `1/${Math.round(1 / seconds)} s`;
}
let simTimer;
async function updateSimulator() {
  const aperture = Number($("aperture").value);
  const shutter = Number($("shutter").value);
  const iso = Number($("iso").value);
  $("apertureValue").textContent = `f/${aperture.toFixed(1).replace(/\.0$/, "")}`;
  $("shutterValue").textContent = formatShutter(shutter);
  $("isoValue").textContent = iso;
  clearTimeout(simTimer);
  simTimer = setTimeout(async () => {
    try {
      const result = await api("/api/simulate", {method:"POST", body:JSON.stringify({aperture, shutter, iso})});
      $("meterValue").textContent = `EV ${result.ev100.toFixed(1)}`;
      $("meterBar").style.width = `${result.brightness}%`;
      $("exposureLabel").textContent = result.brightness > 68 ? "Bright exposure estimate" : result.brightness < 32 ? "Dark exposure estimate" : "Mid-range exposure estimate";
      $("exposureNote").textContent = result.note;
    } catch (error) { $("exposureNote").textContent = error.message; }
  }, 90);
}

async function loadQuiz() {
  try {
    questions = await api("/api/quiz");
    currentQuestion = 0;
    quizScore = 0;
    renderQuestion();
  } catch (error) { $("quizQuestion").textContent = error.message; }
}
function renderQuestion() {
  if (!questions.length) return;
  if (currentQuestion >= questions.length) {
    $("quizCount").textContent = "COMPLETE";
    $("quizQuestion").textContent = `You scored ${quizScore} out of ${questions.length}.`;
    $("quizOptions").innerHTML = "";
    $("quizFeedback").textContent = "Review the lessons, then practise again. Your attempts are saved in your progress dashboard.";
    $("quizScore").textContent = `Final score: ${quizScore}/${questions.length}`;
    $("quizProgressBar").style.width = "100%";
    $("nextQuestion").textContent = "Try again ↻";
    $("nextQuestion").disabled = false;
    $("nextQuestion").dataset.restart = "true";
    return;
  }
  answered = false;
  const q = questions[currentQuestion];
  $("quizCount").textContent = `QUESTION ${currentQuestion + 1} OF ${questions.length}`;
  $("quizQuestion").textContent = q.question;
  $("quizProgressBar").style.width = `${currentQuestion / questions.length * 100}%`;
  $("quizOptions").innerHTML = q.options.map(option => `
    <button class="quiz-option" data-answer="${option.value}">
      <span class="option-letter">${option.value}</span><span>${escapeHtml(option.label)}</span>
    </button>`).join("");
  $("quizOptions").querySelectorAll("[data-answer]").forEach(btn => {
    btn.addEventListener("click", () => submitAnswer(q, btn.dataset.answer, btn));
  });
  $("quizFeedback").textContent = "";
  $("quizScore").textContent = `Score: ${quizScore}`;
  $("nextQuestion").textContent = currentQuestion === questions.length - 1 ? "See results →" : "Next question →";
  $("nextQuestion").disabled = true;
  delete $("nextQuestion").dataset.restart;
}
async function submitAnswer(question, answer, clickedButton) {
  if (answered) return;
  answered = true;
  const buttons = [...$("quizOptions").querySelectorAll("button")];
  buttons.forEach(b => b.disabled = true);
  clickedButton.classList.add("selected");
  try {
    const result = await api("/api/quiz/answer", {method:"POST", body:JSON.stringify({questionId:question.id, answer})});
    if (result.correct) {
      quizScore++;
      clickedButton.classList.add("correct");
      $("quizFeedback").textContent = `That's right. ${result.explanation}`;
    } else {
      clickedButton.classList.add("wrong");
      const correctOption = buttons.find(b => b.dataset.answer === result.correctAnswer);
      if (correctOption) correctOption.classList.add("correct");
      $("quizFeedback").textContent = `Not quite. ${result.explanation}`;
    }
    $("quizScore").textContent = `Score: ${quizScore}`;
    $("nextQuestion").disabled = false;
    updateProgress();
  } catch (error) {
    $("quizFeedback").textContent = error.message;
    $("nextQuestion").disabled = false;
  }
}

async function loadJournal() {
  try {
    const entries = await api("/api/journal");
    $("entryCount").textContent = `${entries.length} ${entries.length === 1 ? "entry" : "entries"}`;
    if (!entries.length) {
      $("journalList").innerHTML = '<p class="empty-state">Your notes will appear here after you save your first entry.</p>';
      return;
    }
    $("journalList").innerHTML = entries.map(e => `
      <article class="journal-entry">
        <div class="entry-top"><h4>${escapeHtml(e.title)}</h4><span class="entry-date">${escapeHtml(e.entry_date)}</span></div>
        ${e.subject ? `<p><strong>Subject:</strong> ${escapeHtml(e.subject)}</p>` : ""}
        ${e.settings ? `<p><strong>Settings:</strong> ${escapeHtml(e.settings)}</p>` : ""}
        ${e.notes ? `<p>${escapeHtml(e.notes)}</p>` : ""}
        <div class="entry-tags">${e.technique ? `<span>${escapeHtml(e.technique)}</span>` : ""}</div>
        <button class="delete-entry" data-delete-entry="${e.id}">Delete entry</button>
      </article>`).join("");
    $("journalList").querySelectorAll("[data-delete-entry]").forEach(btn => btn.addEventListener("click", async () => {
      if (!confirm("Delete this journal entry?")) return;
      try { await api(`/api/journal/${btn.dataset.deleteEntry}`, {method:"DELETE"}); await loadJournal(); showToast("Journal entry deleted."); }
      catch (error) { showToast(error.message); }
    }));
  } catch (error) { $("journalList").innerHTML = `<p class="empty-state">${escapeHtml(error.message)}</p>`; }
}

async function requestAdvice(form) {
  const data = Object.fromEntries(new FormData(form).entries());
  try {
    const result = await api("/api/advisor", {method:"POST", body:JSON.stringify(data)});
    $("advisorResult").innerHTML = `
      <div class="advisor-result-card">
        <span class="tiny-label">YOUR STARTING POINT</span>
        <h3>${escapeHtml(data.subject[0].toUpperCase() + data.subject.slice(1))}</h3>
        <p><strong>Aperture:</strong> ${escapeHtml(result.aperture)}</p>
        <p><strong>Shutter:</strong> ${escapeHtml(result.shutter)}</p>
        <p><strong>ISO:</strong> ${escapeHtml(result.iso)}</p>
        <p>${escapeHtml(result.reason)}</p>
        <p class="warning">${escapeHtml(result.warning)}</p>
      </div>`;
  } catch (error) { $("advisorResult").textContent = error.message; }
}

document.addEventListener("DOMContentLoaded", () => {
  loadLessons();
  updateProgress();
  loadQuiz();
  loadJournal();
  updateSimulator();
  $("entryDate").value = new Date().toISOString().slice(0,10);

  $("lessonFilters").querySelectorAll("[data-filter]").forEach(btn => btn.addEventListener("click", () => {
    $("lessonFilters").querySelectorAll(".filter").forEach(b => b.classList.remove("active"));
    btn.classList.add("active");
    renderLessons(btn.dataset.filter);
  }));
  $("completeLesson").addEventListener("click", async () => {
    if (!activeLesson) return;
    try {
      await api(`/api/lessons/${activeLesson.id}/complete`, {method:"POST"});
      activeLesson.completed = true;
      $("completeLesson").textContent = "Completed ✓";
      $("completeLesson").disabled = true;
      await loadLessons(document.querySelector(".filter.active")?.dataset.filter || "All");
      await updateProgress();
      showToast("Lesson marked complete.");
    } catch (error) { showToast(error.message); }
  });
  ["aperture","shutter","iso"].forEach(id => $(id).addEventListener("input", updateSimulator));
  $("nextQuestion").addEventListener("click", () => {
    if ($("nextQuestion").dataset.restart === "true") {
      $("nextQuestion").dataset.restart = "";
      loadQuiz();
    } else {
      currentQuestion++;
      renderQuestion();
    }
  });
  $("journalForm").addEventListener("submit", async (event) => {
    event.preventDefault();
    const raw = Object.fromEntries(new FormData(event.currentTarget).entries());
    const data = {title:raw.title, entryDate:raw.entryDate, subject:raw.subject, settings:raw.settings, technique:raw.technique, notes:raw.notes};
    try {
      await api("/api/journal", {method:"POST", body:JSON.stringify(data)});
      event.currentTarget.reset();
      $("entryDate").value = new Date().toISOString().slice(0,10);
      $("journalMessage").textContent = "Saved. Your field note is in the journal.";
      await loadJournal();
      showToast("Field note saved.");
    } catch (error) { $("journalMessage").textContent = error.message; }
  });
  $("advisorOpen").addEventListener("click", () => $("advisorDialog").showModal());
  $("advisorForm").addEventListener("submit", event => { event.preventDefault(); requestAdvice(event.currentTarget); });
  document.querySelectorAll("[data-close]").forEach(btn => btn.addEventListener("click", () => $(btn.dataset.close).close()));
  $("mobileMenu").addEventListener("click", () => $("nav").classList.toggle("open"));
  $("nav").querySelectorAll("a").forEach(a => a.addEventListener("click", () => $("nav").classList.remove("open")));
});

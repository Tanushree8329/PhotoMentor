# PhotoMentor 2.0

A beginner-friendly photography learning website with a C++ learning engine, Flask API, and SQLite progress storage.

## Features

- Photography lessons, including exposure, focus, lenses, lighting, white balance, RAW/JPEG, HDR, and composition.
- Composition curriculum covering rule of thirds, golden ratio/spiral, symmetry, leading lines, framing, negative space, patterns, contrast, layers, viewpoint, focal point, shapes, look room, scale, colour, shadows, simplicity, line direction, horizon, subject separation, timing, and aspect ratio.
- Educational exposure simulator.
- Rule-based camera settings advisor.
- Multiple-choice quiz with explanations.
- Persistent lesson completion and quiz statistics.
- Photo journal with create/list/delete.
- Responsive creative website.

## Requirements

- Python 3.10 or newer
- A C++ compiler with C++17 support (`g++` recommended)
- VS Code is recommended but not required

## 1. Open the project

Extract `PhotoMentor2.zip`, then open the `PhotoMentor2` folder in VS Code.

## 2. Compile the C++ engine

Open the VS Code terminal at the project root.

### Windows (MinGW g++)

```powershell
g++ -std=c++17 -O2 cpp_engine/main.cpp -o cpp_engine/photomentor_engine.exe
```

### macOS / Linux

```bash
g++ -std=c++17 -O2 cpp_engine/main.cpp -o cpp_engine/photomentor_engine
```

If `g++` is not recognised, install a C++ compiler and make sure it is on your PATH. On Windows, a MinGW-w64 distribution is a common choice.

## 3. Create and activate a Python virtual environment

### Windows PowerShell

```powershell
py -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install -r requirements.txt
```

If PowerShell blocks activation, use Command Prompt instead:

```bat
.venv\Scripts\activate.bat
python -m pip install -r requirements.txt
```

### macOS / Linux

```bash
python3 -m venv .venv
source .venv/bin/activate
python -m pip install -r requirements.txt
```

## 4. Run the website

```bash
python app.py
```

Open this address in your browser:

http://127.0.0.1:5000

Keep the terminal open while using the website. Press `Ctrl+C` to stop the server.

## 5. Test the C++ engine separately

Windows:

```powershell
.\cpp_engine\photomentor_engine.exe lessons
.\cpp_engine\photomentor_engine.exe questions
.\cpp_engine\photomentor_engine.exe simulate 4 0.008 100
.\cpp_engine\photomentor_engine.exe advisor portrait normal blur
```

macOS/Linux:

```bash
./cpp_engine/photomentor_engine lessons
./cpp_engine/photomentor_engine questions
./cpp_engine/photomentor_engine simulate 4 0.008 100
./cpp_engine/photomentor_engine advisor portrait normal blur
```

The engine returns JSON. Flask calls it and the browser calls Flask's `/api/...` endpoints.

## 6. How the project is connected

1. The browser renders HTML and CSS.
2. JavaScript sends requests to Flask routes such as `/api/lessons` or `/api/simulate`.
3. Flask validates the inputs and invokes the compiled C++ executable.
4. C++ returns JSON containing lesson data, settings recommendations, calculations or quiz results.
5. Flask returns the result to JavaScript.
6. SQLite stores completed lessons, quiz attempts and journal entries.

## Deploy online with Render

This project can be deployed as a Python web service on Render (or a similar host that supports building a C++ executable).

1. Create a Git repository and push the project to GitHub. Do not commit `.venv`, the database, or compiled executables.
2. In Render, create a **New Web Service** and connect the repository.
3. Set the runtime/environment to Python if Render asks.
4. Build command:

```bash
pip install -r requirements.txt && g++ -std=c++17 -O2 cpp_engine/main.cpp -o cpp_engine/photomentor_engine
```

5. Start command:

```bash
gunicorn app:app
```

6. Deploy, then open the public URL Render provides.
7. Check `/health` on the deployed URL; it should return `{"status":"ok"}`.

### Database persistence on hosting

SQLite is fine for a local college demo. Some free hosting instances use an ephemeral filesystem, which means database contents may disappear after a restart or redeploy. For persistent hosted data, configure a persistent disk and set `PHOTOMENTOR_DB` to a database path on that disk, or migrate to a managed database. Do not store private user data in a public repository.

### Important production improvements

- This starter is designed for a student demonstration, not a hardened public service.
- Add user authentication before allowing multiple people to keep private journals.
- Add CSRF protection and stronger date validation before public production use.
- If allowing image uploads later, validate file types and size, store uploads outside public executable paths, and provide deletion controls.
- Do not run Flask's debug server in production; use the provided Gunicorn command.

## Troubleshooting

**“C++ engine not found”**
Compile the engine using the command for your operating system and check that the output file is in `cpp_engine/`.

**`g++` is not recognised**
Install a C++17 compiler and add its `bin` directory to PATH. Restart VS Code after changing PATH.

**Port 5000 is busy**
Windows PowerShell:

```powershell
$env:PORT=5001
python app.py
```

macOS/Linux:

```bash
PORT=5001 python app.py
```

Then open `http://127.0.0.1:5001`.

**Reset the local demo data**
Stop Flask and delete `data/photomentor.db`. The app will create a fresh database the next time it starts. This permanently deletes local progress, quiz history and journal entries.

## Suggested presentation

Demonstrate the homepage, open a lesson, mark it complete, change the exposure simulator settings, ask the advisor for portrait settings, answer a quiz question, then save and delete a journal entry. Explain that C++ supplies the core logic, Flask connects it to the browser, and SQLite persists learning data.

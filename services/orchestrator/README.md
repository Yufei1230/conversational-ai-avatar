# Python backend setup

Install Python 3.10 or newer (the reference setup used Python 3.11). From the repository root, run in PowerShell:

```powershell
cd services/orchestrator
py -3.11 -m venv .venv
.\.venv\Scripts\python.exe -m pip install -e .
if (-not (Test-Path .env)) { Copy-Item .env.example .env }
```

For Bash, use `python3 -m venv .venv`, `.venv/bin/python -m pip install -e .`, and `cp -n .env.example .env`. Run backend commands from `services/orchestrator` so the service loads its local `.env`.

## Configure the providers

Start from [`.env.example`](.env.example). Enter your own credentials and verify that your provider account has access to the configured models. Keep `.env` local.

| Stage | Settings to check |
|---|---|
| NVIDIA Riva ASR | `ACE_ASR_SERVER`, `ACE_ASR_FUNCTION_ID`, `ACE_ASR_USE_HOSTED_API`, `ACE_ASR_USE_SSL`. If `ACE_ASR_API_KEY` is empty, ASR uses `ACE_NIM_API_KEY`. |
| Hosted LLM | `ACE_NIM_API_KEY`, `ACE_NIM_LLM_BASE_URL`, `ACE_NIM_LLM_MODEL`. The checked-in default is Nemotron 3.5 Lightning. Test a real completion when changing models. |
| NVIDIA Magpie TTS | `ACE_TTS_USE_HOSTED_API`, `ACE_TTS_API_URL`, `ACE_TTS_VOICE`. The hosted adapter uses `ACE_NIM_API_KEY`. |
| Languages | `ASSISTANT_LANGUAGE`, `ASR_LANGUAGE`, `TTS_LANGUAGE`. Start with the English defaults and select a matching TTS voice. |

Keep `ACE_MOCK_ASR`, `ACE_MOCK_LLM`, and `ACE_MOCK_TTS` false for a real provider run. Set `ACE_SAVE_DEBUG_AUDIO=false` if you do not need server-side audio recordings.

## Start the service

```powershell
.\.venv\Scripts\python.exe -m uvicorn app.main:app --host 127.0.0.1 --port 8080 --workers 1
```

The observer registry is in memory and requires one worker. This command keeps the unauthenticated endpoint on loopback. In another terminal, `Invoke-RestMethod http://127.0.0.1:8080/healthz` checks liveness, and `Invoke-RestMethod http://127.0.0.1:8080/status` shows the provider configuration. A full request in the [run guide](../../docs/RUN.md) verifies inference.

Run backend tests with `.\.venv\Scripts\python.exe -m unittest discover -s tests -v`. For self-hosted services, see the optional [deployment notes](../../infra/compose/README.md).

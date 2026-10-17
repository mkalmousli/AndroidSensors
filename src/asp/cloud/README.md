# cloud

`asp cloud build|status|logs|fetch|cleanup`, using your logged-in `gcloud` and `gh`.

- `setup`: enables APIs, stores the GitHub token in Secret Manager, grants the VM's service account access
- `vm`: creates the temporary build VM (runs `/cloud/startup.py` at boot)
- `commands`, `cli`, `tools`: actions, argument parsing, `gcloud`/`gh` wrappers

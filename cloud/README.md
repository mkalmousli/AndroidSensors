# cloud

`startup.py` is the build VM's startup script. It installs Docker, clones the repo, builds the image,
runs `asp release` (publishes to GitHub; on failure publishes the log instead) and always deletes the VM.

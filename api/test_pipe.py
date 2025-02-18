import time
import os

scripts = ["generated/podcast_1739831587/script.json", 
           "generated/podcast_1739826494/script.json", 
           "generated/podcast_1739770953/script.json", 
           "generated/podcast_1739829695/script.json"]

def pipe_script(script_path):
    try:
        with open("../engine_pipe", "w") as pipe:
            pipe.write(f"api/{script_path}" + "\n")
        print(f"Wrote {script_path} to pipe")
    except Exception as e:
        print(f"Failed to write to engine pipe: {e}")

if __name__ == "__main__":
    while True:
        for script in scripts:
            pipe_script(script)
            time.sleep(60)  # Wait 60 seconds between scripts

import subprocess

Import("env")

def get_git_version():
    try:
        # Run the git describe command
        result = subprocess.run(
            ["git", "describe", "--dirty", "--always", "--tags"],
            capture_output=True,
            text=True,
            check=True
        )
        # Return the stripped output
        return result.stdout.strip()
    except Exception as e:
        # If git fails, return a default version
        print(f"Git version check failed: {e}")
        return "0.0.0-dev"

# Get the version
firmware_version = get_git_version()

# Print the version for verification during build
print(f"Firmware Version: {firmware_version}")

# Append the version as a build flag (C macro)
env.Append(
    BUILD_FLAGS=[
        f'-DAPP_VERSION=\\"{firmware_version}\\"'
    ]
)

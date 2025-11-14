#!/bin/bash

# --- START CONFIGURATION ---
# !!! 1. IMPORTANT: YOU MUST CHANGE THIS !!!
#
#    Run 'conda info --base' in your *working* terminal.
#    Paste the path it gives you here.
#    (This is an EXAMPLE, replace it with your real path)
#
CONDA_BASE_PATH="/home/xuqizhi/anaconda3"

# !!! 2. CHANGE THIS if your env is not 'base' !!!
#
#    (Run 'conda env list' to check)
#
CONDA_ENV_NAME="base"
# --- END CONFIGURATION ---


# --- Do not edit below this line ---

# Check if the path is set
if [ "${CONDA_BASE_PATH}" == "/home/xuqizhi/anaconda3" ]; then
    echo "WARNING: CONDA_BASE_PATH is set to the default example path."
    echo "Please edit this script and set it to your real Conda path."
    echo "(Find it by running 'conda info --base' in your terminal)"
fi

# Find the conda init script
CONDA_INIT_SCRIPT="${CONDA_BASE_PATH}/etc/profile.d/conda.sh"

# Check if the init script exists
if [ ! -f "${CONDA_INIT_SCRIPT}" ]; then
    echo "ERROR: Conda init script not found at:"
    echo "${CONDA_INIT_SCRIPT}"
    echo "Please make sure CONDA_BASE_PATH is set correctly in this script."
    read -p "Press [Enter] to exit..."
    exit 1
fi

echo "Sourcing Conda script: ${CONDA_INIT_SCRIPT}"
source "${CONDA_INIT_SCRIPT}"

echo "Activating Conda environment: ${CONDA_ENV_NAME}..."
conda activate "${CONDA_ENV_NAME}"

if [ "$CONDA_DEFAULT_ENV" != "$CONDA_ENV_NAME" ]; then
    echo "ERROR: Failed to activate Conda env: ${CONDA_ENV_NAME}"
    read -p "Press [Enter] to exit..."
    exit 1
fi

echo "Conda env activated: ${CONDA_DEFAULT_ENV}"

# Change to the script's directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
echo "Changing directory to: ${SCRIPT_DIR}"
cd "${SCRIPT_DIR}"

# Run your Python script
echo "Running: python test_overed.py"
python test_overed.py

EXIT_CODE=$?
echo "---"
echo "Script finished with exit code: ${EXIT_CODE}"

# (Keeps the terminal open for debugging)
echo "Script finished. Press [Enter] to close this terminal..."
read
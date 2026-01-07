# Check if inside a Git repo
if (-not (Test-Path ".git")) {
    Write-Host "❌ Not a Git repository."
    exit 1
}

# Prompt for commit message
$Message = Read-Host "Enter your commit message"

# Stage all changes
git add src/main.c

# Commit with the entered message
git commit -m "$Message"

Write-Host "✅ Changes committed with message: $Message"

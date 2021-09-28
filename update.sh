#!/bin/bash
# Simple git update script

# Get name of dir for currrent week
result=${PWD##*/}

# add all files
git add *

# Commit msg
git commit -m "Update $result"

# Push to repo
git push -u origin main
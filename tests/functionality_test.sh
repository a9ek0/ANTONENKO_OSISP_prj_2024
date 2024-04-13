#!/bin/bash

# Clear the console
clear

# Change to the mnt directory
echo "Changing to the mnt directory..."
cd /home/alexander/mnt

# Create the qwe directory
echo "Creating the qwe directory..."
mkdir testDir

# Change to the qwe directory
echo "Changing to the qwe directory..."
cd testDir

# Create the testFile.txt file
echo "Creating the testFile.txt file..."
touch testFile.txt

# Reading directory
echo "Reading directory..."
dir

# Write "Test" to the testFile.txt file
echo "Writing 'Test' to the testFile.txt file..."
echo "Test" >> testFile.txt

# Display the contents of the testFile.txt file
echo "Contents of the testFile.txt file:"
cat testFile.txt

# Open the testFile.txt file
echo "Opening the testFile.txt file..."
open testFile.txt

# Display a script completion message
echo "Script completed"
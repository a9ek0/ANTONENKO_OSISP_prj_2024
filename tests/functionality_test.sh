#!/bin/bash

# Clear the console
clear

# Change to the mnt directory
echo "Changing to the mnt directory..."
cd /home/alexander/mnt

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

# Create the test directory
echo "Creating the test directory..."
mkdir testDir

# Change to the qwe directory
echo "Moving testFile.txt to the test directory..."
mv testFile.txt testDir

# Change to the testDir directory
echo "Changing to the testDir directory..."
cd testDir

# Display the contents of the testFile.txt file
echo "Contents of the testFile.txt file:"
cat testFile.txt

# Change to the qwe directory
echo "Renaming testFile.txt to testFile2.txt ..."
mv testFile.txt testFile2.txt

# Write "Test" to the testFile.txt file
echo "Writing 'Test' to the testFile2.txt file..."
echo "Test" >> testFile2.txt

# Open the testFile2.txt file
echo "Opening the testFile2.txt file..."
open testFile2.txt

# Display a script completion message
echo "Script completed"
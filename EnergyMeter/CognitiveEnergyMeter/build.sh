#!/bin/bash
echo 'clean'
make clean

echo 'building'
make

echo 'binary copied to: '$1  
cp ./Debug/CognitiveEnergyMeter $1

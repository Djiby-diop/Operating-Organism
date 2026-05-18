#!/bin/bash
IMG='/mnt/c/Users/djibi/OneDrive/Bureau/baremetal/llm-baremetal/llm-baremetal-boot.img'
echo "--- GPT header ---"
gdisk -l "$IMG" 2>&1 | head -25
echo "--- models dir ---"
mdir -i "${IMG}@@1048576" ::models/ 2>&1
echo "--- repl.cfg ---"
mtype -i "${IMG}@@1048576" ::repl.cfg 2>&1 || mtype -i "${IMG}@@1048576" ::REPL.CFG 2>&1 || echo "(no repl.cfg)"
echo "--- llmk-autorun.txt ---"
mtype -i "${IMG}@@1048576" ::llmk-autorun.txt 2>&1 || echo "(no autorun)"

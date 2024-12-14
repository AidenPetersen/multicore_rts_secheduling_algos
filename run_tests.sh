for f in $(ls tests/*);
do
    echo "Test: $f"
    cat $f | ./llref_emu
done
echo "OFF" | ./lab4b >stdout.txt 2>stderr.txt
if [ $? -eq 0 ]
then
    echo "OK on OFF test"
else
    echo "Error 1: Wrong return code in OFF command"
fi

./lab4b --bogus >stdout.txt 2>stderr.txt
if [ $? -ne 0 ]
then
    echo "OK on invalid argument check"
else
    echo "Error 2: Wrong return code for invalid arguments"
fi

echo "OFF" | ./lab4b --log=out.txt >stdout.txt 2>stderr.txt
if [ ! -s out.txt ]
then 
    echo "Error 3: Cannot create log file"
else
    echo "OK in creating log files"
fi
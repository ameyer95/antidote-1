
DEPTH=1
START=0
END=1542
FILENAME=output-compas.json

touch output-compas.json
chmod u+x output-compas.json

for I in $(seq $START $END)
do
    bin/main -f data compas -d $DEPTH -t $I >> $FILENAME
done

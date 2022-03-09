# this script generates some plot

PLOT=fig12


for d in */;
do
	if [ -f "$d/data/uname" ]; then
		echo $d
		python3 ../plotgen/$PLOT*.py $d/data/$PLOT $d/$PLOT.pdf
	fi
done

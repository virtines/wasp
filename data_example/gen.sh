echo "# Data Examples"

for d in */;
do
	if [ -d "$d/data" ]; then
		echo "## [$d](https://github.com/virtines/wasp/tree/main/data_example/$d)"
		echo '- Software: ' $(cat "$d/data/uname")
		echo '- Hardware:'
		echo '```' 
		cat "$d/data/cpuinfo" | head -n 28
		echo '```'
	fi
done

# This script generates the README.md file in the current directory.
# This is because we're somewhat lazy :^)

echo "# Data Examples"
echo "This directory contains various artifacts generated on a wide variety"
echo "of processors and microarchitectures. Software/Hardware details are below:"

for d in */;
do
	if [ -d "$d/data/uname" ]; then
		echo "## [$d](https://github.com/virtines/wasp/tree/main/data_example/$d)"
		echo '- Software: ' $(cat "$d/data/uname")
		echo '- Hardware:'
		echo '```' 
		cat "$d/data/cpuinfo" | head -n 28
		echo '```'
	fi
done

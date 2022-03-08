


./bombardier -c 100 -n 10000 \
	-b '{ "vargs": {"string":"Hello"} }' \
	-H 'Content-Type: application/json' \
	-m POST \
	'http://localhost:8989/actions/jsn/invoke'

exit

while true; do
	curl --location --request POST 'http://localhost:8989/actions/jsn/invoke' \
		--header 'Content-Type: application/json'                         \
		--data-raw '{ "vargs": {"string":"Hello"} }';
done

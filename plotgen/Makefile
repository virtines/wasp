PYTHON:=python3
all: abs-latency.pdf abs-latency-with-creation.pdf wasp-latency.pdf mode-latency.pdf unikern-boot.pdf echo-latency.pdf virtine-latency.pdf virtine-vs-lambda.pdf http-latency.pdf http-tput.pdf marshalling-costs.pdf

abs-latency.pdf: abs-latency.py ../data/abstraction_latencies/*.csv
	$(PYTHON) $< $@

abs-latency-with-creation.pdf: abs-latency-with-creation.py ../data/abstraction_latencies/*.csv
	$(PYTHON) $< $@

wasp-latency.pdf: wasp-latency.py ../data/abstraction_latencies/*.csv
	$(PYTHON) $< $@

mode-latency.pdf: mode-latency.py ../data/fib-20/*.csv
	$(PYTHON) $< $@

unikern-boot.pdf: unikern-boot.py ../data/boottime/*.csv
	$(PYTHON) $< $@

echo-latency.pdf: echo-latency.py ../data/boottime/echo-server.csv
	$(PYTHON) $< $@

boot-milestones.pdf: boot-milestones.py ../data/boottime/echo-server.csv
	$(PYTHON) $< $@

virtine-latency.pdf: virtine-latency.py ../data/rust_frontend/fib/*.csv
	$(PYTHON) $< $@

dataflow-local.pdf: dataflow-local.py ../data/rust_frontend/df/*.csv
	$(PYTHON) $< $@

virtine-vs-lambda.pdf: virtine-vs-lambda.py ../data/rust_frontend/df/*.csv ../data/aws_lambda/*.csv
	$(PYTHON) $< $@

http-latency.pdf: http-latency.py ../data/http/*.csv
	$(PYTHON) $< $@

http-tput.pdf: http-tput.py ../data/http/*.csv
	$(PYTHON) $< $@

js.pdf: js.py ../data/js/*.csv
	$(PYTHON) $< $@
	
openssl.pdf: openssl.py ../data/openssl/*.csv
	$(PYTHON) $< $@

marshalling-costs.pdf: marshalling-costs.py ../../wasp/data/*.csv
	$(PYTHON) $< $@


image-size.pdf: image-size.py ../data/imagesize.csv
	$(PYTHON) $< $@

fib-latency.pdf: fib-latency.py ../../wasp/data/fib/tinker-2.cs.iit.edu/*.csv
	python3 $< $@

clean:
	rm -f *.pdf


.PHONY: clean all

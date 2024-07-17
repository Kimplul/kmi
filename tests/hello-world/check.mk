REPORT	:= reports/hello-world
hello-world: do-hello-world
	@grep 'Hello, world!' $(REPORT)/log > /dev/null	\
		&& echo "OK" > $(REPORT)/OK		\
		|| echo "ERR" > $(REPORT)/ERR

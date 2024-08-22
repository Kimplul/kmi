fork: do-fork
	@grep 'BUG' reports/fork/log \
		&& echo 'BUG' > reports/fork/OK \
		|| tail -n1 reports/fork/log | tr -d '\r' > reports/fork/OK

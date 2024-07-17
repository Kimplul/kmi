noop: do-noop
	@# remove carriage returns before writing status
	@tail -n1 reports/noop/log | tr -d '\r' > reports/noop/OK

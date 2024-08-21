ipc-req: do-ipc-req
	@grep 'BUG' reports/ipc-req/log \
		&& echo 'BUG' > reports/ipc-req/OK \
		|| tail -n1 reports/ipc-req/log | tr -d '\r' > reports/ipc-req/OK

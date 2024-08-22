ipc-notify: do-ipc-notify
	@grep 'BUG' reports/ipc-notify/log \
		&& echo 'BUG' > reports/ipc-notify/OK \
		|| tail -n1 reports/ipc-notify/log | tr -d '\r' > reports/ipc-notify/OK

malloc: do-malloc
	@grep 'BUG' reports/malloc/log \
		&& echo 'BUG' > reports/malloc/OK \
		|| tail -n1 reports/malloc/log | tr -d '\r' > reports/malloc/OK

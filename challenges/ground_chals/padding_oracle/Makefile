
containers:
	docker build . -t local/padding:latest
	docker run -p 10002:8000 -v ~/Finals/padding_oracle/test_keys:/keys -it local/padding:latest
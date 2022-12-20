REGISTRY ?= scrubbed/astropoint

build: 

	docker build --no-cache --build-arg TOKEN=${GITLAB_TOKEN}  -t $(REGISTRY)/challenge:latest .
start_rabbit:
	docker run -d \
			--rm \
			--hostname test-rabbit \
			--name test-rabbit \
			-e RABBITMQ_DEFAULT_USER=test \
			-e RABBITMQ_DEFAULT_PASS=passwd \
			-p 10000:5672 \
			-p 10001:15672 \
			scrubbed/rabbit_mq
	sleep 20

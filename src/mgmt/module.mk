mgmt:

GRPC_FLAGS := -I src/mgmt/proto/def --python_out=src/mgmt/proto/ --grpc_python_out=src/mgmt/proto/

.PHONY: proto
proto:
	python3 -m grpc_tools.protoc $(GRPC_FLAGS) src/mgmt/proto/def/*.proto

clean::
	rm -rf src/mgmt/proto/__pycache__
	find src/mgmt/proto/ -name '*.py' -not -name '__init__.py' -delete
module := zeta

submodules := tests
-include $(patsubst %, $(module)/%/module.mk, $(submodules))

GRPC_FLAGS := -I zeta/proto/ --python_out=. --grpc_python_out=.

.PHONY: proto
proto:
	python3 -m grpc_tools.protoc $(GRPC_FLAGS) zeta/proto/zeta/proto/*.proto

clean::
	rm -rf zeta/proto/__pycache__
	find zeta/proto/ -name '*.py' -not -name '__init__.py' -delete
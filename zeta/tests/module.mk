mgmt_tests:: all
	sudo python3 -W ignore -m unittest discover zeta/tests/ -v

.PHONY: run_mgmt_test
run_mgmt_test:
	sudo python3 -W ignore -m unittest zeta.tests.${TEST} -v

clean::
	rm -rf zeta/test/__pycache__
	rm -rf zeta/test/*.pyc
	rm -rf zeta/common/__pycache__
	rm -rf zeta/common/*.pyc

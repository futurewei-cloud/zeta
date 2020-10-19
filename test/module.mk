# SPDX-License-Identifier: GPL-2.0-or-later

test::
	sudo python3 -W ignore -m unittest discover test/ -v

.PHONY:
run_unittests: unittest
	echo running unittest
	find ./build/tests/ -type f -name "test_*" -exec {} \;


.PHONY: run_mgmt_test
run_mgmt_test:
	sudo python3 -W ignore -m unittest zeta.tests.${TEST} -v

clean::
	rm -rf zeta/test/__pycache__
	rm -rf zeta/test/*.pyc
	rm -rf zeta/common/__pycache__
	rm -rf zeta/common/*.pyc
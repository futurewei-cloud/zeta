# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Sherif Abdelwahab <@zasherif>
#          Phu Tran          <@phudtran>

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:The above copyright
# notice and this permission notice shall be included in all copies or
# substantial portions of the Software.THE SOFTWARE IS PROVIDED "AS IS",
# WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
# TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
# FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
# THE USE OR OTHER DEALINGS IN THE SOFTWARE.

import luigi
from workflows.common.bootstrap import *
from workflows.droplets.create import *
from workflows.droplets.provisioned import *
from workflows.droplets.delete import *

from workflows.dfts.create import *
from workflows.dfts.provisioned import *
from workflows.dfts.delete import *

from workflows.chains.create import *
from workflows.chains.provisioned import *
from workflows.chains.delete import *

from workflows.ftns.create import *
from workflows.ftns.provisioned import *
from workflows.ftns.delete import *


class ZetaWorkflowFactory():

    def DropletCreate(self, param):
        return DropletCreate(param=param)

    def DropletProvisioned(self, param):
        return DropletProvisioned(param=param)

    def DropletDelete(self, param):
        return DropletDelete(param=param)

    def DftCreate(self, param):
        return DftCreate(param=param)

    def DftProvisioned(self, param):
        return DftProvisioned(param=param)

    def DftDelete(self, param):
        return DftDelete(param=param)

    def ChainCreate(self, param):
        return ChainCreate(param=param)

    def ChainProvisioned(self, param):
        return ChainProvisioned(param=param)

    def ChainDelete(self, param):
        return ChainDelete(param=param)

    def FtnCreate(self, param):
        return FtnCreate(param=param)

    def FtnProvisioned(self, param):
        return FtnProvisioned(param=param)

    def FtnDelete(self, param):
        return FtnDelete(param=param)

    def CommonOperatorStart(self, param):
        return CommonOperatorStart(param=param)

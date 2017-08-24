from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

import sys
import itertools

import google.protobuf.text_format

import torch.jit
from torch.autograd import Variable

import toffee
from toffee.backend import Caffe2Backend as c2


try:
    import caffe2
except ImportError:
    print('Cannot import caffe2, hence caffe2-torch test will not run.')
    sys.exit(0)

torch.set_default_tensor_type('torch.FloatTensor')
try:
    import torch
except ImportError:
    print('Cannot import torch, hence caffe2-torch test will not run.')
    sys.exit(0)

if torch.cuda.is_available():
    def toC(x):
        return x.cuda()
else:
    def toC(x):
        return x


def torch_export(model, x, proto_init=False):
    # Enable tracing on the model
    trace, torch_out = torch.jit.record_trace(toC(model), toC(x))
    if proto_init is False:
        proto = torch._C._jit_pass_export(trace)
    else:
        proto = torch._C._jit_pass_export(trace, toC(model).state_dict().values())
    return proto, torch_out


def caffe2_load(proto, model, x, state_dict=None):

    graph_def = toffee.GraphProto.FromString(proto)
    # TODO: This is a hack; PyTorch should set it
    graph_def.version = toffee.GraphProto().version

    toffee.checker.check_graph(graph_def)

    # Translate the parameters into Caffe2 form
    W = {}
    if state_dict:
        parameters = []
        # Passed in state_dict may have a different order.  Make
        # sure our order is consistent with the model's order.
        # TODO: Even better: keyword arguments!
        for k in model.state_dict():
            parameters.append(state_dict[k])
    else:
        parameters = model.state_dict().values()
    for k, v in zip(graph_def.input, itertools.chain(parameters, [x])):
        # On C2 side, we don't run on CUDA yet so convert to CPU memory
        if isinstance(v, Variable):
            W[k] = v.data.cpu().numpy()
        else:
            W[k] = v.cpu().numpy()

    caffe2_out_workspace = c2.run_graph(
        init_graph=None,
        predict_graph=graph_def,
        inputs=W)
    caffe2_out = list(caffe2_out_workspace.values())[0]
    return caffe2_out

#!/usr/bin/env python2

import sys
sys.path.append("@PYTHON_INSTALL_DIR@")

import zmq
from optparse import OptionParser
from mvp.MVPMessages_pb2 import MVPCommand
from mvp.MVPWorkspaceRequest_pb2 import MVPWorkspaceRequest

parser = OptionParser()
parser.usage = "%prog [options] <mvpd server>"
parser.add_option("--launch", dest="cmd_launch_filename", help="Start a batch", metavar="mvp.conf")
parser.add_option("--abort",  dest="cmd_abort",     help="Abort current batch", action="store_true")
parser.add_option("--status", dest="cmd_status",    help="Print the status of the current batch", action="store_true")

(options, args) = parser.parse_args()

if len(args) != 1:
  parser.print_help()
  exit(1)

if not(options.cmd_abort or options.cmd_status or options.cmd_launch_filename):
  parser.print_help()
  exit(1)

cmd_sock_url = args[0]
timeout = 1000

context = zmq.Context()
cmd_sock = context.socket(zmq.REQ)
cmd_sock.connect(cmd_sock_url)
poller = zmq.Poller()
poller.register(cmd_sock, zmq.POLLIN)

if options.cmd_abort:
  cmd = MVPCommand()
  cmd.cmd = MVPCommand.ABORT
  cmd_sock.send(cmd.SerializeToString())
  evts = poller.poll(timeout)
  if not(evts):
    print "Failed to receive reply from", cmd_sock_url

if options.cmd_status:
  cmd = MVPCommand()
  cmd.cmd = MVPCommand.STATUS
  cmd_sock.send(cmd.SerializeToString())
  evts = poller.poll(timeout)
  if not(evts):
    print "Failed to receive reply from", cmd_sock_url

if options.cmd_launch_filename:
  cmd = MVPCommand()
  cmd.cmd = MVPCommand.LAUNCH
  cmd.conf_file = options.cmd_launch_filename
  cmd_sock.send(cmd.SerializeToString())
  evts = poller.poll(timeout)
  if not(evts):
    print "Failed to receive reply from", cmd_sock_url

cmd_sock.linger = 0
cmd_sock.close()
context.term()

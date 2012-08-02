#!/usr/bin/env python2

import sys
sys.path.insert(0, "@PYTHON_INSTALL_DIR@")

import zmq
from optparse import OptionParser
from mvp.frontend.Messages_pb2 import CommandMsg, CommandReplyMsg
from mvp.frontend.SessionDesc_pb2 import SessionDesc
from mvp.core.GlobalSettings_pb2 import GlobalSettings
from google.protobuf import text_format

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

mvp_settings = GlobalSettings()

cmd_sock_url = "tcp://" + args[0] + ":" + mvp_settings.ports.command
timeout = 1000

context = zmq.Context()
cmd_sock = context.socket(zmq.REQ)
cmd_sock.connect(cmd_sock_url)
poller = zmq.Poller()
poller.register(cmd_sock, zmq.POLLIN)

def sendCommand(cmd):
  cmd_sock.send(cmd.SerializeToString())
  evts = poller.poll(timeout)
  if not(evts):
    print "Failed to receive reply from", cmd_sock_url

cmd = CommandMsg()

if options.cmd_abort:
  cmd.cmd = CommandMsg.ABORT
  sendCommand(cmd)

if options.cmd_status:
  cmd.cmd = CommandMsg.STATUS
  sendCommand(cmd)

if options.cmd_launch_filename:
  cmd.cmd = CommandMsg.LAUNCH
  session_desc = open(options.cmd_launch_filename, 'r')
  text_format.Merge(session_desc.read(), cmd.session)
  sendCommand(cmd)

cmd_sock.linger = 0
cmd_sock.close()
context.term()

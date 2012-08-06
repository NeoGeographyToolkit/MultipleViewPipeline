#!/usr/bin/env python2

import sys
sys.path.insert(0, "@PYTHON_INSTALL_DIR@")

import zmq
from optparse import OptionParser
from mvp.frontend.Messages_pb2 import CommandMsg, CommandReplyMsg, StatusReport
from mvp.frontend.SessionDesc_pb2 import SessionDesc
from mvp.core.GlobalSettings_pb2 import GlobalSettings
from google.protobuf import text_format
from mvp.pipeline.JobDesc_pb2 import JobDesc

parser = OptionParser()
parser.usage = "%prog [options] <mvpd server>"
parser.add_option("--launch", dest="cmd_launch_filename", help="Start a batch", metavar="mvp.conf")
parser.add_option("--abort",  dest="cmd_abort",     help="Abort current batch", action="store_true")
parser.add_option("--kill",  dest="cmd_kill",     help="Kill workers", action="store_true")
parser.add_option("--status", dest="cmd_status",    help="Print the status of the current batch", action="store_true")

(options, args) = parser.parse_args()

if len(args) != 1:
  parser.print_help()
  exit(1)

if not(options.cmd_abort or options.cmd_status or options.cmd_kill or options.cmd_launch_filename):
  parser.print_help()
  exit(1)

mvp_settings = GlobalSettings()

cmd_sock_url = "tcp://" + args[0] + ":" + mvp_settings.ports.command
timeout = mvp_settings.timeouts.command

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
    return 0
  reply = CommandReplyMsg()
  reply.ParseFromString(cmd_sock.recv())
  return reply

def printStatusReport(status_report):
  print "Completed jobs: {0} of {1}".format(status_report.jobs_completed, status_report.total_jobs)
  print "\nActive jobs:"
  for i in status_report.actives:
    print "{0}, {1} @{2}: {3}%".format(i.job.render.col, i.job.render.row, i.job.render.level, i.status * 100)
  if (status_report.orphans):
    print "\nOrphaned Jobs:"
    for i in status_report.orphans:
      print "{0}, {1} @{2}".format(i.render.col, i.render.row, i.render.level)
    

cmd = CommandMsg()

if options.cmd_abort:
  cmd.cmd = CommandMsg.ABORT
  sendCommand(cmd)

if options.cmd_kill:
  cmd.cmd = CommandMsg.KILL
  sendCommand(cmd)

if options.cmd_status:
  cmd.cmd = CommandMsg.STATUS
  reply = sendCommand(cmd)
  if reply:
    printStatusReport(reply.status_report)

if options.cmd_launch_filename:
  cmd.cmd = CommandMsg.LAUNCH
  session_desc = open(options.cmd_launch_filename, 'r')
  text_format.Merge(session_desc.read(), cmd.session)
  sendCommand(cmd)

cmd_sock.linger = 0
cmd_sock.close()
context.term()

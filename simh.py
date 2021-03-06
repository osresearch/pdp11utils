import pexpect
import os.path
import time
import sys

class SIMH:
    def __init__(self,simh_path,init_path):
        simh_path = os.path.expanduser(simh_path)
        self.p = pexpect.spawn('{0} {1}'.format(simh_path,init_path))
        time.sleep(0.5)
        self.stopped = False
        self.stop()

    def cont(self):
        if (self.stopped):
            self.p.send('cont\n')
            time.sleep(0.1)
            self.p.send('\n')
            self.p.expect('.')
            self.stopped = False

    def stop(self):
        if (not self.stopped):
            self.p.send('\x05')
            self.p.expect('sim> ')
            self.stopped = True


    def load_address(self,address):
        self.addr = address

    def deposit_word(self, w):
        self.stop()
        self.p.send("d {0} {1}\n".format(oct(self.addr),oct(w)))
        self.p.expect('sim> ')
        self.addr = self.addr + 2

    def start(self, addr):
        self.stop()
        self.p.send("g {0}\n".format(oct(addr)))

    def wait_for(self,s,timeout = 200):
        sys.stderr.write("WAITING FOR {0}".format(s))
        sys.stderr.flush()
        self.p.expect(s,timeout)

    def send(self,s):
        bs = 1024
        for i in range(0,len(s),bs):
            self.p.send(s[i:i+bs])

    def read(self,sz):
        rsp = ''
        while len(rsp) < sz:
            rsp = rsp + self.p.read_nonblocking(sz-len(rsp),10)
        return rsp

    def send_file(self,native_path,remote_path):
        self.stop()
        self.p.send('attach ptr {0}\n'.format(native_path))
        self.cont()
        self.do('COPY PC: {0}\n'.format(remote_path))
        self.p.expect('.')

    def recv_file(self,remote_path,native_path):
        self.stop()
        self.p.send('attach ptp {0}\n'.format(native_path))
        self.cont()
        self.do('COPY {0} PC:\n'.format(remote_path))
        self.p.expect('.')

    def do(self,command,timeout=1):
        self.cont()
        self.p.send('{0}\n'.format(command))
        rsp = ''
        try:
            while True:
                rsp = rsp + self.p.read_nonblocking(1024,timeout)
        except:
            pass
        return rsp


if __name__=='__main__':
    import argparse
    import os
    parser = argparse.ArgumentParser()
    parser.add_argument("--simh-path",type=str,default=None,
                        help="Path to pdp11 SIMH binary")
    parser.add_argument("--init-script",type=str,default='./default.ini',
                        help="Path to SIMH init script (should boot rt-11)")
    parser.add_argument("--send",action='append',default=[],
                        help="Send file to RT-11 (format: 'local=remote')")
    parser.add_argument("--recv",action='append',default=[],
                        help="Recieve file from RT-11 (format: 'local=remote')")
    parser.add_argument("--do",action='append',default=[],
                        help="Do command on RT-11")

    args = parser.parse_args()
    if not args.simh_path:
        args.simh_path = os.environ.get('SIMH_PATH','~/opt/bin/pdp11')
    s = SIMH(args.simh_path,args.init_script)
    for path in args.send:
        if path.find("=") >= 0:
            local,remote = path.split("=",2)
        else:
            local,remote = path,path
        s.send_file(local,remote)
    for cmd in args.do:
        s.do(cmd)
    for path in args.recv:
        if path.find("=") >= 0:
            remote,local = path.split("=",2)
        else:
            remote,local = path,path
        s.recv_file(remote,local)


    
    

#!/usr/local/bin/python3

import os
import sys
import time
import json
import signal

# Stolen from: https://stackoverflow.com/questions/18499497/how-to-process-sigterm-signal-gracefully
class GracefulKiller:
    kill_now = False
    signum = None
    def __init__(self):
        signal.signal(signal.SIGINT, self.exit_gracefully)
        signal.signal(signal.SIGTERM, self.exit_gracefully)

    def exit_gracefully(self,signum, frame):
        self.kill_now = True
        self.signum = signum

if __name__ == '__main__':

    # Create file for liveness probe.
    os.system('touch /app/healthy')

    # Wait for system to check.
    time.sleep(5)

    # Create file for readiness probe.
    os.system('touch /app/ready')
    
    # Setup dict for yaml file.
    yamloutput = {
        'ext_ch10_mnt_point': None,
        'ch10_file_name': None,
        'ext_icd_mnt_point': None,
        'icd_file_name': None,
        'overwrite': False,
        'video': False,
        }

    req_arg_count = 7
    if(len(sys.argv) < req_arg_count + 1):
        print('Not enough args!')
        os.system('echo FAIL > /app/termination-log')
        sys.exit(0)

    out_file_name = 'data.json'
    out_path = os.path.join('/app', out_file_name)

    # Set output values.
    yamloutput['ext_ch10_mnt_point'] = sys.argv[1]
    yamloutput['ch10_file_name'] = sys.argv[2]
    yamloutput['ext_icd_mnt_point'] = sys.argv[3]
    yamloutput['icd_file_name'] = sys.argv[4]
    yamloutput['overwrite'] = sys.argv[5]
    yamloutput['video'] = sys.argv[6]
    tfstr = sys.argv[7]
    kill_switch = False
    if tfstr.lower() in ['true', 't', '1', 'yes', 'y']:
        kill_switch = True

    # Write yaml file.
    with open(out_path, 'w') as f:
        json.dump(yamloutput, f)
    #print(yaml.dump(yamloutput) + '\n')

    with open('/app/ext_ch10_mnt_point', 'w') as f:
        f.write(yamloutput['ext_ch10_mnt_point'])
    with open('/app/ext_icd_mnt_point', 'w') as f:
        f.write(yamloutput['ext_icd_mnt_point'])

    reps = 5
    wait = 2
    ind = 0
    killer = GracefulKiller()
    while ind < reps:
        if killer.kill_now:
            print('Caught signal: {:d}'.format(killer.signum))
            break
        
        #print('waiting ....')
        time.sleep(wait)
        ind += 1

        if kill_switch == True:
            print('Kill switch engaged!')
            sys.exit(1)

    # debug
    os.system('cat {:s}'.format(out_path))

    # termination message
    os.system('echo OK > /app/termination-log')

    # not ready
    os.system('rm /app/ready')

    time.sleep(1)

    # Remove liveness probe check.
    os.system('rm /app/healthy')
    sys.exit(0)

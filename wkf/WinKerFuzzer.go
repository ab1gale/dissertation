package main

import (
"encoding/json"
"flag"
"fmt"
//"io/ioutil"
"log"
"os"
//"os/exec"
//"reflect"
"time"
"github.com/wkf/vm"
)
//Suppose we have same pair of username and password for all vms
type Config struct{
	//global: instance do not need to access
	Http			string
	Vm_count		int
	//env: for data that is same for every instance but need to be accessed by each
	Env				*vm.Environ 
	//snapshot: each vmware instance has its own snapshot
	Snapshots		[]vm.Snapshot
}
type Manager struct{
	Cfg 		*Config
	Start_time	time.Time
	Fuzz_time	time.Duration
}
type Crash struct{
	Vm_idx		int
	Brief		string
	Report		[]byte		
}

var flagConfig=flag.String("config", "", "config file")

func main(){
	
	flag.Parse()
	//file, _ := os.Open("conf.json")
	CfgFile, err := os.Open(*flagConfig)
	defer CfgFile.Close()
	Jsondecoder := json.NewDecoder(CfgFile)
	var cfg Config
	err = Jsondecoder.Decode(&cfg)
	if err != nil {
  	log.Fatalf("error: %v while reading config", err)

	}
	cfg.Vm_count=len(cfg.Snapshots)
	mgr:=&Manager{
		Cfg:		&cfg,
		Start_time:	time.Now(),
	}
	mgr.mgrLoop()

}
type Run_result struct{
	Id   int
	Cr *Crash
	Err   error
}
func (mgr *Manager) mgrLoop(){
	fmt.Println("Starting & Waiting for connections...")
	inst_per_repro := 2
	vcount := mgr.Cfg.Vm_count
	if inst_per_repro > vcount {
		inst_per_repro = vcount
	}
	instances := make([]int, vcount)
	// Index of instance 
	for i := range instances {
		instances[i] = i
	}
	run_done := make(chan *Run_result, 1)
	//repro_in_queue := make(map[*Crash]bool)
	//reproducing := make(map[string]bool)

	//TODO: need a loop 2 deal with repro

	for{
		for len(instances) !=0 {
			first_id := vcount-len(instances)
			if first_id != vcount-1 {
				instances = instances[first_id+1:]
			} else{
				instances=nil
			}
			fmt.Printf("Starting instance %v\n",first_id)
			go func(){
				crash, err := mgr.runInstance(first_id)
				time.Sleep(2*time.Second)
				run_done <- &Run_result{first_id,crash,err}
			}()
		}
	}
	
}
func (mgr *Manager) runInstance(idx int)(*Crash, error) {
	if idx < 0 || idx >= mgr.Cfg.Vm_count {
		return nil, fmt.Errorf("Invalic vm idx %v", idx)
	}
	inst,err := mgr.Cfg.Snapshots[idx].Start(mgr.Cfg.Env,idx)
	inst.Copy("/Users/ab1gale/go/src/github.com/wkf/guest/fuzzer/device.py","C:\\Users\\ab1gale\\Desktop\\device.py")
	return nil, err
}

package vm
import(
//"fmt"
"log"
"os/exec"
"time"
)
type Environ struct{
	Vmrun_path	string
	Host_type	string
	Username	string
	Password	string
	Workdir		string
	Proc_num	int
	Listening_port	int
}
type Instance struct{
	Idx			int
	Snap 		*Snapshot
	Env 		*Environ
}
type Snapshot struct{
	System			string
	Word_length		int
	Vm_path			string
	Snapshot_name	string
}

func (snap *Snapshot) Start(env *Environ,idx int)(*Instance,error){
	args_revert := []string{
		"-T",env.Host_type,
		"revertToSnapshot",
		snap.Vm_path,
		snap.Snapshot_name,
	}
	args_start := []string{
		"-T",env.Host_type,
		"start",
		snap.Vm_path,
	}
	revert_snap:=exec.Command(env.Vmrun_path,args_revert...)
	revert_snap.Run()
	time.Sleep(2*time.Second)
	start_vm:=exec.Command(env.Vmrun_path,args_start...)
	start_vm.Run()
	time.Sleep(8*time.Second)
	return &Instance{
		Idx:	idx,
		Snap:	snap,
		Env:	env,
	},nil
}

func (inst *Instance) Copy(path_host string,path_guest string){
	fmt.Printf("Copying file %v to guest...\n",path_host)
	args_copy:=[]string{
		"-T",inst.Env.Host_type,
		"-gu",inst.Env.Username,
		"-gp",inst.Env.Password,
		"CopyFileFromHostToGuest",
		inst.Snap.Vm_path,
		path_host,path_guest,
	}
	copy_file:=exec.Command(inst.Env.Vmrun_path,args_copy...)
	//copy_file.Run()
	_,err:=copy_file.CombinedOutput()
	if err != nil {
		log.Fatal(err)
	}
	time.Sleep(2*time.Second)
}

func (inst *Instance) Run(){
	
}
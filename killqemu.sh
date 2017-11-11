kill -2 `ps -ef|grep qemu-system|grep -v grep|awk '{print $2}' `

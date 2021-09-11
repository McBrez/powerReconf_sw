connect -url tcp:127.0.0.1:3121
targets -set -nocase -filter {name =~ "*microblaze*#0" && bscan=="USER2" }
loadhw -hw D:/DEV/powerReconf_sw/soc_proj_platform/export/soc_proj_platform/hw/bd_soc_proj_wrapper.xsa -regs
configparams mdm-detect-bscan-mask 2
targets -set -nocase -filter {name =~ "*microblaze*#0" && bscan=="USER2" }
rst -system
after 3000
targets -set -nocase -filter {name =~ "*microblaze*#0" && bscan=="USER2" }
dow D:/DEV/powerReconf_sw/soc_proj_app_2/Debug/soc_proj_app_2.elf
bpadd -addr &main

cmd_/home/team5/SHD-SpectreLab/module-src/Module.symvers := sed 's/\.ko$$/\.o/' /home/team5/SHD-SpectreLab/module-src/modules.order | scripts/mod/modpost -m -a  -o /home/team5/SHD-SpectreLab/module-src/Module.symvers -e -i Module.symvers   -T -

#!/bin/csh

# Synchronize for RRAM-based FPGA at nominal voltage 
set rhrs_list = (10 20 30 40 50 60 70 80 90 100 500)
foreach j ($rhrs_list)
  # Duplicate 
  cp k6_N10_rram_tsmc40nm.conf k6_N10_rram_tsmc40nm_RHRS$j\MOhm.conf
  # replace with the new numbers 
  sed -i 's/RHRS100MOhm/RHRS'${j}'MOhm/g' k6_N10_rram_tsmc40nm_RHRS$j\MOhm.conf
end

# Synchronize for RRAM-based FPGA at near-Vt regime 
set rhrs_list = (100 500 30 20)
foreach j ($rhrs_list)
  # Duplicate 
  cp k6_N10_rram_tsmc40nm_nearVt0p8V.conf k6_N10_rram_tsmc40nm_nearVt0p8V_RHRS$j\MOhm.conf
  # replace with the new numbers 
  sed -i 's/RHRS100MOhm/RHRS'${j}'MOhm/g' k6_N10_rram_tsmc40nm_nearVt0p8V_RHRS$j\MOhm.conf
end

set rhrs_list = (100 500 30 20)
foreach j ($rhrs_list)
  # Duplicate 
  cp k6_N10_rram_tsmc40nm_nearVt0p7V.conf k6_N10_rram_tsmc40nm_nearVt0p7V_RHRS$j\MOhm.conf
  # replace with the new numbers 
  sed -i 's/RHRS100MOhm/RHRS'${j}'MOhm/g' k6_N10_rram_tsmc40nm_nearVt0p7V_RHRS$j\MOhm.conf
end

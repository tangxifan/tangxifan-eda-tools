#!/bin/csh

# Synchronize for RRAM-based FPGA at nominal voltage 
set rhrs_list = (10 20 30 40 50 60 70 80 90 100 500)
foreach j ($rhrs_list)
  # Duplicate 
  cp k6_N10_rram_tsmc40nm.xml k6_N10_rram_tsmc40nm_RHRS$j\MOhm.xml
  # replace with the new numbers 
  sed -i 's/roff=\"100e6\"/roff=\"'${j}'e6\"/g' k6_N10_rram_tsmc40nm_RHRS$j\MOhm.xml
end

# Synchronize for RRAM-based FPGA at near-Vt regime 
set rhrs_list = (100 500 30 20)
foreach j ($rhrs_list)
  # Duplicate 
  cp k6_N10_rram_tsmc40nm_nearVt0p8V.xml k6_N10_rram_tsmc40nm_nearVt0p8V_RHRS$j\MOhm.xml
  # replace with the new numbers 
  sed -i 's/roff=\"100e6\"/roff=\"'${j}'e6\"/g' k6_N10_rram_tsmc40nm_nearVt0p8V_RHRS$j\MOhm.xml
end

set rhrs_list = (100 500 30 20)
foreach j ($rhrs_list)
  # Duplicate 
  cp k6_N10_rram_tsmc40nm_nearVt0p7V.xml k6_N10_rram_tsmc40nm_nearVt0p7V_RHRS$j\MOhm.xml
  # replace with the new numbers 
  sed -i 's/roff=\"100e6\"/roff=\"'${j}'e6\"/g' k6_N10_rram_tsmc40nm_nearVt0p7V_RHRS$j\MOhm.xml
end

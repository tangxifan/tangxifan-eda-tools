# Run High-Performance SiNWFET LIB
#perl dc_flow.pl -conf conf/NW_HP.conf -benchmark verilog/benchmark.txt -rpt NW_HP.csv -binary_search
#perl dc_flow.pl -conf conf/NW_HP.conf -benchmark verilog/benchmark_NWHP.txt -rpt NW_HP.csv -reduce_error_to_warning
# Run Low-leakage SiNWFET LIB
#perl dc_flow.pl -conf conf/NW_LL.conf -benchmark verilog/benchmark.txt -rpt NW_LL.csv -binary_search
perl dc_flow.pl -conf conf/NW_LL.conf -benchmark verilog/benchmark_NWLP.txt -rpt NW_LL.csv -reduce_error_to_warning
# Run High-Performance and Low-leakage MIXED SiNWFET LIB
#perl dc_flow.pl -conf conf/NW_ALL.conf -benchmark verilog/benchmark.txt -rpt NW_ALL.csv -binary_search
#perl dc_flow.pl -conf conf/NW_ALL.conf -benchmark verilog/benchmark_NWALL.txt -rpt NW_ALL.csv -reduce_error_to_warning
# Run FinFET
#perl dc_flow.pl -conf conf/FinFET.conf -benchmark verilog/benchmark_FinFET.txt -rpt FinFET.csv -binary_search -reduce_error_to_warning
#perl dc_flow.pl -conf conf/FinFET.conf -benchmark verilog/benchmark_FinFET.txt -rpt FinFET_new.csv -reduce_error_to_warning


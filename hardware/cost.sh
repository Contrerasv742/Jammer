# Brief: Parsed a CSV files Quantiy and Cost row to give a total
#   @param $1 File to Be parsed
awk -F',' 'NR>1 && $3 != "" { total += $3 * $6 } END { printf "Total: $%.2f\n", total }' $1

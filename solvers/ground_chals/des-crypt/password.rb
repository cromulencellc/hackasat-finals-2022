# Replace password with one found by solve.py
password = 'syndromesworedisplacerepaintcaretaker'
station='Melbourne'

payload =  "{\"password\":\"#{password}\"}"

# Pick a groundstation
cmd("GS SELECT_STATION with OPCODE 1, STATION_NAME "+station)

# Access request
cmd("GS ACCESS_REQUEST with OPCODE 2, STATION_NAME "+station+", PAYLOAD "+payload)
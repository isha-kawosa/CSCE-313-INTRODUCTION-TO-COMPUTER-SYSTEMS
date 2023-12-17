from scapy.all import *

pktsent= False

def main():
    global pktsent
    """Driver function"""
    while True:
        print_menu()
        option = input('Choose a menu option: ')
        if option == '1':
            print("Creating and sending packets ...")
            num_packets = int(input("How many packets? "))
            interval = int(input("How many seconds in between sending? "))
            send_pkt(num_packets, interval)
            pktsent= False
        elif option == '2':
            print("Listening to all traffic to 8.8.4.4 for 1 minute ...")
            sniff(filter="dst host 8.8.4.4", timeout=60, prn=print_pkt)
        elif option == '3':
            print("Listening continuously to only ping commands to 8.8.4.4 ...")
            sniff(filter="icmp and dst host 8.8.4.4", prn=print_pkt)
        elif option == '4':
            print("Listening continuously to only outgoing telnet commands ...")
            sniff(filter="tcp and dst port 23", prn=print_pkt, store=0)
        elif option == '5':
            print("End")
            break
        else:
            print(f"\nInvalid entry\n")


def send_pkt(number, interval):
    global pktsent
    """
    Send a custom packet with the following fields

    #### Ethernet layer
    - Source MAC address: 00:11:22:33:44:55
    - Destination MAC address: 55:44:33:22:11:00

    #### IP layer
    - Source address: 192.168.10.4
    - Destination address: 8.8.4.4
    - Protocol: TCP
    - TTL: 26

    #### TCP layer
    - Source port: 23
    - Destination port: 80

    #### Raw payload
    - Payload: "RISC-V Education: https://riscvedu.org/"
    """

    # packet = Ether(src="00:11:22:33:44:55", dst="55:44:33:22:11:00") / \
    #         IP(src="192.168.10.4", dst="8.8.4.4", ttl=26) / \
    #         TCP(sport=23, dport=80) / \
    #         Raw(load="RISC-V Education: https://riscvedu.org/")
    ether_layer = Ether(src="00:11:22:33:44:55", dst="55:44:33:22:11:00") 
    ip = IP(src="192.168.10.4", dst="8.8.4.4", ttl=26) 
    tcp = TCP(sport=23, dport=80) 
    payload= "RISC-V Education: https://riscvedu.org/"
    packet= ether_layer/ ip/ tcp/ Raw(load=payload)
    sendp(packet, count =number, inter=interval)
    pktsent = True


def print_pkt(packet):
    """ 
    Print Packet fields

    - Source IP
    - Destination IP
    - Protocol number
    - TTL
    - Length in bytes
    - Raw payload (if any)
    """

    print(f"Source IP: {packet[IP].src}")
    print(f"Destination IP: {packet[IP].dst}")
    print(f"Protocol number: {packet[IP].proto}")
    print(f"TTL: {packet[IP].ttl}")
    print(f"Length in bytes: {len(packet)}")
    if Raw in packet:
        print(f"Raw payload: {packet[Raw].load}")
    else:
        print("Raw payload: None")

    print()


def print_menu():
    """Prints the menu of options"""
    print("*******************Main Menu*******************")
    print('1. Create and send packets')
    print('2. Listen to all traffic to 8.8.4.4 for 1 minute')
    print('3. Listen continuously to only ping commands to 8.8.4.4')
    print('4. Listen continuously to only outgoing telnet commands')
    print('5. Quit')
    print('***********************************************\n')


main()
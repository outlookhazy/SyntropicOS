*** Keywords ***
Create Multinode Network
    Execute Command         include @${CURDIR}/syn_multinode.resc

*** Test Cases ***
Multinode Router and Heartbeat Communication
    [Documentation]         Boots two simulated STM32F4 nodes connected via a UART hub,
    ...                     verifies mutual heartbeat discovery, and tests end-to-end
    ...                     reliable routing of application data packets.
    Create Multinode Network

    ${tester0}=             Create Terminal Tester  sysbus.usart2   timeout=6   machine=node-0
    ${tester1}=             Create Terminal Tester  sysbus.usart2   timeout=6   machine=node-1

    Start Emulation

    # Both nodes should discover each other via heartbeat broadcasts
    Wait For Line On Uart   Peer 2 found!   testerId=${tester0}
    Wait For Line On Uart   Peer 1 found!   testerId=${tester1}

    # Node 1 should successfully send the app packet to Node 2
    Wait For Line On Uart   Node 2 received: Hello Node 2!   testerId=${tester1}

#pragma once

void rx_init();
void rx_interrupt_init();
char rx_read();

void tx_init();
char tx_send(char c);
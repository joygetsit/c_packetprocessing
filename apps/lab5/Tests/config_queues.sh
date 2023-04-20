#!/bin/bash

nfp-reg mecsr:i32.me0.Mailbox{0..3}

nfp-reg mecsr:i32.me0.Mailbox0=0 mecsr:i32.me0.Mailbox1=8 mecsr:i32.me0.Mailbox2=16

nfp-reg mecsr:i32.me0.Mailbox{0..3}

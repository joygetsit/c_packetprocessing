#!/bin/bash


nfp-reg mecsr:i32.me0.Mailbox0=0 mecsr:i32.me0.Mailbox1=8 mecsr:i32.me0.Mailbox2=8

nfp-reg xpb:Nbi0IsldXpbMap.NbiTopXpbMap.TrafficManager.TMShaperReg.ShaperRate{{0..3},144}.Rate=0x3fff
nfp-reg xpb:Nbi0IsldXpbMap.NbiTopXpbMap.TrafficManager.TMShaperReg.ShaperRate{0..1}.Rate=1
nfp-reg xpb:Nbi0IsldXpbMap.NbiTopXpbMap.TrafficManager.TrafficManagerReg.TrafficManagerConfig.ShaperEnable=1

# IP pools

This document outlines the IP pools used in production and lab/testing

## Production

### Google IP pools

**Global**


**Per-rack**

Rack | VIP type | VIPs
--- | --- | ---
IAD1 | UDP/GRE tunnel | 64.9.242.96, 2604:ca00:f003:3::0
IAD1 | API-server | 64.9.242.107, 2604:ca00:f003:3::b
IAD2 | UDP/GRE tunnel | 64.9.242.160, 2604:ca00:f003:4::0
IAD2 | API-server | 64.9.242.171, 2604:ca00:f003:4::b
LAX1 | UDP/GRE tunnel | 136.22.70.96, 2604:ca00:f011:2::0
LAX1 | API-server | 136.22.70.107, 2604:ca00:f011:2::b
LAX2 |UDP/GRE tunnel | 136.22.70.160, 2604:ca00:f011:4::0
LAX2 | API-server | 136.22.70.171, 2604:ca00:f011:4::b
LGA1 | UDP/GRE tunnel | 64.9.243.96, 2604:ca00:f004:3::0
LGA1 | API-server | 64.9.243.107, 2604:ca00:f004:3::b
LGA2 | UDP/GRE tunnel | 64.9.243.160, 2604:ca00:f004:4::0
LGA2 | API-server | 64.9.243.171, 2604:ca00:f004:4::b
ORD1 | UDP/GRE tunnel | 64.9.241.96, 2604:ca00:f002:3::0
ORD1 | API-server | 64.9.241.107, 2604:ca00:f002:3::b
ORD2 | UDP/GRE tunnel | 64.9.241.160, 2604:ca00:f002:4::0
ORD2 | API-server | 64.9.241.171, 2604:ca00:f002:4::b
SEA1 | UDP/GRE tunnel | 136.22.92.96, 2604:ca00:f00c:2::0
SEA1 | API-server | 136.22.92.107, 2604:ca00:f00c:2::b
SEA2 | UDP/GRE tunnel | 136.22.92.224, 2604:ca00:f00c:102::0
SEA2 | API-server | 136.22.92.235, 2604:ca00:f00c:102::b
SJC1 | UDP/GRE tunnel | 64.9.240.96, 2604:ca00:f001:3::0
SJC1 | API-server | 64.9.240.107, 2604:ca00:f001:3::b
SJC2 | UDP/GRE tunnel | 64.9.240.160, 2604:ca00:f001:4::0
SJC2 | API-server | 64.9.240.171, 2604:ca00:f001:4::b

### Partner IP pools

Category | IPs
--- | ---
GW IPv6 | 2620:134:b000::0/40
Cloud (private cluster) | 35.227.135.188/30, 35.223.251.42/32, 192.31.242.0/24
Cloud (public cloud) | 13.72.26.23, 13.72.26.223
CDR/NBR SFTP-server | 35.230.181.207

## Lab/testing

### Google IP pools

Rack | VIP type | VIPs
--- | --- | ---
Nuq13 | UDP/GRE tunnel | 136.22.65.96, 2604:ca00:f:61::0
Nuq13 | API-server |  136.22.65.107, 2604:ca00:f:61::b

### Partner IP pools

TODO
# GnuPG

## 1. create keys

```
[root@centos7u5 ~]# gpg2 --gen-key
gpg (GnuPG) 2.0.22; Copyright (C) 2013 Free Software Foundation, Inc.
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.

Please select what kind of key you want:
   (1) RSA and RSA (default)
   (2) DSA and Elgamal
   (3) DSA (sign only)
   (4) RSA (sign only)
Your selection?
RSA keys may be between 1024 and 4096 bits long.
What keysize do you want? (2048)
Requested keysize is 2048 bits
Please specify how long the key should be valid.
         0 = key does not expire
      <n>  = key expires in n days
      <n>w = key expires in n weeks
      <n>m = key expires in n months
      <n>y = key expires in n years
Key is valid for? (0)
Key does not expire at all
Is this correct? (y/N) y

GnuPG needs to construct a user ID to identify your key.

Real name: Zhiyuan Dai
Email address: jerry.zhiyuan.dai@gmail.com
Comment:
You selected this USER-ID:
    "Zhiyuan Dai <jerry.zhiyuan.dai@gmail.com>"

Change (N)ame, (C)omment, (E)mail or (O)kay/(Q)uit? O
You need a Passphrase to protect your secret key.

We need to generate a lot of random bytes. It is a good idea to perform
some other action (type on the keyboard, move the mouse, utilize the
disks) during the prime generation; this gives the random number
generator a better chance to gain enough entropy.
We need to generate a lot of random bytes. It is a good idea to perform
some other action (type on the keyboard, move the mouse, utilize the
disks) during the prime generation; this gives the random number
generator a better chance to gain enough entropy.
gpg: key D0CBFCE9 marked as ultimately trusted
public and secret key created and signed.

gpg: checking the trustdb
gpg: 3 marginal(s) needed, 1 complete(s) needed, PGP trust model
gpg: depth: 0  valid:   1  signed:   0  trust: 0-, 0q, 0n, 0m, 0f, 1u
pub   2048R/D0CBFCE9 2021-02-02
      Key fingerprint = ED54 E5A5 4590 A811 1FBC  3D02 0A67 1B12 D0CB FCE9
uid                  Zhiyuan Dai <jerry.zhiyuan.dai@gmail.com>
sub   2048R/5811BFE2 2021-02-02

[root@centos7u5 ~]# gpg2 --list-keys
/root/.gnupg/pubring.gpg
------------------------
pub   2048R/D0CBFCE9 2021-02-02
uid                  Zhiyuan Dai <jerry.zhiyuan.dai@gmail.com>
sub   2048R/5811BFE2 2021-02-02
```
- public key: /root/.gnupg/pubring.gpg
- private key: /root/.gnupg/secring.gpg

```

Backup Keys:
```
-----BEGIN PGP PUBLIC KEY BLOCK-----
Version: GnuPG v2.0.22 (GNU/Linux)

mQENBGAYxVkBCADDZm2UlPUEGZlnlnWdg3v/sCpvUe/ls1DxK78eZp8p9tz7/fiB
KPZp+flQx1t54eKdSFa1e3reer/D3AYgH5Pve7L7BxRN8/9AMjF/gZxNJ980+KZ0
KcLzwCISJjUhsAItc+IE1s/+gXdUSl0LzHdos0UPySAHniDDZlKmYLTSoErBFk4D
Enw7m+iLHuZ6YEFqhC1YU4Jd5DbGhydjIvAc5XzFtSQKLCc95Op1E0/eHDFzryhW
K6zh/uQCYeNz6pnkV8qgfU2iyJXpD+ERj9f5Bk3DWpVRYWkbHuvhucQ6srrAckk6
y/0wQPnSUfROY+bfPNxjRMsM/yYzFGoVklBpABEBAAG0KVpoaXl1YW4gRGFpIDxq
ZXJyeS56aGl5dWFuLmRhaUBnbWFpbC5jb20+iQE5BBMBAgAjBQJgGMVZAhsDBwsJ
CAcDAgEGFQgCCQoLBBYCAwECHgECF4AACgkQCmcbEtDL/Ono+Qf+O4fw5cwHfr+F
aljkP9WUqBsamimzC8VziKR3Bf7DJMAdFYBdYSJ/hAxRFkc4LU5oL6oxw17wtKci
osj80x8bcNVqkelWqJpuIoR8Ay7oZDz8YdnYHfIO2XWs2aXhuf7O8vBRC1cEpo7q
2XjwZ0oFfVrIFWVHDGtqb/44VSqlXs1HqDhcVcq17V8YWOU04UeX+HXtHrnQVboe
LaNZZHvsqkGKAEjt0Z/626wTKlKtzFc/UrkfBUGRZ+kl2C4P0mOJA4B3mSBdHq1s
i0xMtGydwyK0EuprNgn5V9VWqMwZaM/dv7mFOxz1hCffv/YYHwKz40U17QJEw2jn
SxrhpKapRbkBDQRgGMVZAQgA5ZRKZGQfFqFlGxLweEitcKNKcBCY5B5OVILVjbVX
iueQDxsBUTh3gCAhffm8+usE8b5EVaMKnlPc/UqMOwFTmTH06hQaue2+MEOBtAC0
BDh/ZXcEd+DMgZllmr4Xh6+u390r8YvF8u9o+AmiSEUt4KPPeqZqM5YYs7iJ+AuB
wG+wgA+GVaY7jwCxyqpLSR+bium86zTIrY3yI3XHkcWnYEUSPz4DkQ+0S63WkuS8
arhlSETEk6aPgLHCbwwLeYvAg4DoJB+2gQHGGKyobPqgx1qcmteP4WUNYWdZ8n1P
K+kfBHYTZZMQqlUgIHRNoE/8QJkvoN085vl3nQs99Cm7nwARAQABiQEfBBgBAgAJ
BQJgGMVZAhsMAAoJEApnGxLQy/zpZkcH/25G6YWLW6ZPK/M7x3pHvxvodoiizPVb
xymDNWbjCGAiFm7FMu0HMhSnFAHap4GxTZl+DQBKdzIdxL02VqK18cT94WOTQvuX
T3OgFnCOTxLrXLJzwAENE6oJeDHoQK+sXmu9WKXKqWyh4uv7lN79M8caEAUYR41o
c1AzK9Tm8bzsbHQFWje+RGDJMFz1OzF02epmBiXtSCv7GDS/z2vGWVopjcvTCxE+
y/jT/MzshiCpI8touFZz/C6q5CHU9NOT8z4jjN+b/q1sAY15vi5jYUh83BOAdn0W
iZ5nod3uu3a1jtcEfNItUdJqEUDO9g8LW6WIMnk6/JMDX1LBvMOAF28=
=z1Oy
-----END PGP PUBLIC KEY BLOCK-----
```

```
-----BEGIN PGP PRIVATE KEY BLOCK-----
Version: GnuPG v2.0.22 (GNU/Linux)

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
LVHSahFAzvYPC1uliDJ5OvyTA19SwbzDgBdv
=pGaW
-----END PGP PRIVATE KEY BLOCK-----
```

## 2. export keys
```
[root@centos7u5 ~]# gpg2 --output public.pgp --armor --export "Zhiyuan Dai <jerry.zhiyuan.dai@gmail.com>"
[root@centos7u5 ~]# gpg2 --output private.pgp --armor --export-secret-key "Zhiyuan Dai <jerry.zhiyuan.dai@gmail.com>"
```

## 3. delete keys
```
[root@centos7u5 ~]# gpg2 --delete-secret-keys  "Zhiyuan Dai <jerry.zhiyuan.dai@gmail.com>"
[root@centos7u5 ~]# gpg2 --delete-keys  "Zhiyuan Dai <jerry.zhiyuan.dai@gmail.com>"
```

## 4. import keys
```
[root@centos7u5 ~]# gpg2 --import public.pgp
[root@centos7u5 ~]# gpg2 --import private.pgp
```

## 5. Key Server

```
[root@centos7u5 ~]# gpg2 --fingerprint --list-keys
/root/.gnupg/pubring.gpg
------------------------
pub   2048R/D0CBFCE9 2021-02-02
      Key fingerprint = ED54 E5A5 4590 A811 1FBC  3D02 0A67 1B12 D0CB FCE9
uid                  Zhiyuan Dai <jerry.zhiyuan.dai@gmail.com>
sub   2048R/5811BFE2 2021-02-02

[root@centos7u5 ~]# gpg --send-keys D0CBFCE9
gpg: sending key D0CBFCE9 to hkp server keys.gnupg.net

[root@centos7u5 ~]# gpg --search-keys "Zhiyuan Dai"
gpg: searching for "Zhiyuan Dai" from hkp server keys.gnupg.net
(1)     Zhiyuan Dai <jerry.zhiyuan.dai@gmail.com>
          2048 bit RSA key D0CBFCE9, created: 2021-02-02
```

## 6. Encrypt & Decrypt

Encrypt:
```
[root@centos7u5 ~]# gpg2 --encrypt --armor --recipient "Zhiyuan Dai <jerry.zhiyuan.dai@gmail.com>" SecretDocment.txt
gpg: 5811BFE2: There is no assurance this key belongs to the named user

pub  2048R/5811BFE2 2021-02-02 Zhiyuan Dai <jerry.zhiyuan.dai@gmail.com>
 Primary key fingerprint: ED54 E5A5 4590 A811 1FBC  3D02 0A67 1B12 D0CB FCE9
      Subkey fingerprint: A4DD E242 7B0D 36C4 1FCE  2F1C 29C9 1EAD 5811 BFE2

It is NOT certain that the key belongs to the person named
in the user ID.  If you *really* know what you are doing,
you may answer the next question with yes.

Use this key anyway? (y/N) y

[root@centos7u5 tmp]# cat SecretDocment.txt.asc
-----BEGIN PGP MESSAGE-----
Version: GnuPG v2.0.22 (GNU/Linux)

hQEMAynJHq1YEb/iAQf8DvwaBeZpUyB4isBudUdlOp1PYkMnHFofpSQXiVR1v1Jk
vhlgNq7/n5q8AgH+La/PxJMV/LoBEZzef1N7QjFE3Gqa1shd+jKx7DBqniyu+fBV
PSrQKSjbGXnnKJE5G4DDtOTRNIAiZhEiVWo20nM1EWFvIZmLRYx/jLD8ZstacZm+
il6efEME7s5rYAHbNH9v8Azy50gaVxgjnou+L/fDHXkDt2xV/pabEMvvNCFuvxXC
w3YDV8EQ0dC9fCZlUqXaGbpdKuEhAemr41CZjVF/OYk8bxbBUSF8dXaf5eKdRdV7
5TQ1P+SyAVhEjCLHMmO885RfNrLPynCLHOKMHLFSe9JmAR+JLIwcakfeVUAfruAI
XmgVzuZXt2Z5es9dDWyzickhGSLK0Aj0UEAWIlUkalaPnt1U81OUHMuwdHz6O+gN
gOJGMnKDewe10udFlMM89glKHz6Cvt5RYiU2oziXrG2t0LIuijQJ
=/GOd
-----END PGP MESSAGE-----
```
Decrypt:
```
[root@centos7u5 ~]# gpg2 SecretDocment.txt.asc

You need a passphrase to unlock the secret key for
user: "Zhiyuan Dai <jerry.zhiyuan.dai@gmail.com>"
2048-bit RSA key, ID 5811BFE2, created 2021-02-02 (main key ID D0CBFCE9)

gpg: encrypted with 2048-bit RSA key, ID 5811BFE2, created 2021-02-02
      "Zhiyuan Dai <jerry.zhiyuan.dai@gmail.com>"
[root@centos7u5 ~]# cat SecretDocment.txt
it was a very nice story.
```

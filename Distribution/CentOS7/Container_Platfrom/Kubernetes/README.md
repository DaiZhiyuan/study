 - 1 使用Minikube配置单节点Kubernetes集群
    - 1.1 配置虚拟化环境
    - 1.2 配置Kubernetes源并安装Minikube
    - 1.3 创建删除Pods
- 2 使用Kubeadm配置多节点Kubernetes集群
    - 2.1 在所有节点上安装Kubeadm
    - 2.2 配置集群
    - 2.3 将工作节点加入到集群
    - 2.4 使用持久存储
    - 2.5 使用私有仓库

Kubernetes是Docker的编排系统。

# 1. 使用Minikube配置单节点Kubernetes集群

## 1.1 配置虚拟化环境

```Shell
[root@lab ~]# yum -y install qemu-kvm libvirt libvirt-daemon-kvm

[root@lab ~]# systemctl start libvirtd 
[root@lab ~]# systemctl enable libvirtd 
```

## 1.2 配置Kubernetes源并安装Minikube

```Shell
[root@lab ~]# cat <<'EOF' > /etc/yum.repos.d/kubernetes.repo
[kubernetes]
name=Kubernetes
baseurl=https://packages.cloud.google.com/yum/repos/kubernetes-el7-$basearch
enabled=1
gpgcheck=1
repo_gpgcheck=1
gpgkey=https://packages.cloud.google.com/yum/doc/yum-key.gpg https://packages.cloud.google.com/yum/doc/rpm-package-key.gpg
EOF

[root@lab ~]# yum -y install kubectl
[root@lab ~]# wget https://storage.googleapis.com/minikube/releases/latest/minikube-linux-amd64 -O minikube 
[root@lab ~]# wget https://storage.googleapis.com/minikube/releases/latest/docker-machine-driver-kvm2 
[root@lab ~]# chmod 755 minikube docker-machine-driver-kvm2 
[root@lab ~]# mv minikube docker-machine-driver-kvm2 /usr/local/bin/
[root@lab ~]# minikube version 
minikube version: v0.25.2

[root@lab ~]# kubectl version -o json 
{
  "clientVersion": {
    "major": "1",
    "minor": "10",
    "gitVersion": "v1.10.0",
    "gitCommit": "fc32d2f3698e36b93322a3465f63a14e9f0eaead",
    "gitTreeState": "clean",
    "buildDate": "2018-03-26T16:55:54Z",
    "goVersion": "go1.9.3",
    "compiler": "gc",
    "platform": "linux/amd64"
  }
}

# start minikube
[root@lab ~]# minikube start --vm-driver kvm2 
Starting local Kubernetes v1.9.4 cluster...
Starting VM...
Downloading Minikube ISO
 142.22 MB / 142.22 MB  100.00% 0s
Getting VM IP address...
Moving files into cluster...
Downloading localkube binary
 163.02 MB / 163.02 MB  100.00% 0s
 65 B / 65 B  100.00% 0s
Setting up certs...
Connecting to cluster...
Setting up kubeconfig...
Starting cluster components...
Kubectl is now configured to use the cluster.
Loading cached images from config file.

# show status
[root@lab ~]# minikube status 
minikube: Running
cluster: Running
kubectl: Correctly Configured: pointing to minikube-vm at 192.168.39.110

[root@lab ~]# minikube service list 
|-------------|----------------------|-----------------------------|
|  NAMESPACE  |         NAME         |             URL             |
|-------------|----------------------|-----------------------------|
| default     | kubernetes           | No node port                |
| kube-system | kube-dns             | No node port                |
| kube-system | kubernetes-dashboard | http://192.168.39.110:30000 |
|-------------|----------------------|-----------------------------|

[root@lab ~]# minikube docker-env 
export DOCKER_TLS_VERIFY="1"
export DOCKER_HOST="tcp://192.168.39.110:2376"
export DOCKER_CERT_PATH="/root/.minikube/certs"
export DOCKER_API_VERSION="1.23"
# Run this command to configure your shell:
# eval $(minikube docker-env)

[root@lab ~]# kubectl cluster-info 
Kubernetes master is running at https://192.168.39.110:8443

To further debug and diagnose cluster problems, use 'kubectl cluster-info dump'.

[root@lab ~]# kubectl get nodes 
NAME       STATUS    ROLES     AGE       VERSION
minikube   Ready     <none>    1h        v1.9.4

# a VM [minikube] is just running
[root@lab ~]# virsh list 
 Id    Name                           State
----------------------------------------------------
 1     minikube                       running
 
# possible to access with SSH to the VM
[root@lab ~]# minikube ssh 
                         _             _
            _         _ ( )           ( )
  ___ ___  (_)  ___  (_)| |/')  _   _ | |_      __
/' _ ` _ `\| |/' _ `\| || , <  ( ) ( )| '_`\  /'__`\
| ( ) ( ) || || ( ) || || |\`\ | (_) || |_) )(  ___/
(_) (_) (_)(_)(_) (_)(_)(_) (_)`\___/'(_,__/'`\____)

$ hostname 
minikube

$ docker ps 
CONTAINER ID IMAGE                             COMMAND                CREATED     STATUS  PORTS NAMES
7d769e956904 fed89e8b4248                      "/sidecar --v=2 --..." 3 hours ago Up 3 ho       k8s_si..
39680ff90ecf 459944ce8cc4                      "/dnsmasq-nanny -v..." 3 hours ago Up 3 ho       k8s_dn..
e568a2ac6088 512cd7425a73                      "/kube-dns --domai..." 3 hours ago Up 3 ho       k8s_ku..
fd9bd248d82f e94d2f21bc0c                      "/dashboard --inse..." 3 hours ago Up 3 ho       k8s_ku..
f2c59bf5dd0a gcr.io/k8s-minikube/storage-prer  "/storage-provisioner" 3 hours ago Up 3 ho       k8s_st..
203ac12b2138 gcr.io/google_containers/pause3.0 "/pause"               3 hours ago Up 3 ho       k8s_PO..
6d88074f01dc gcr.io/google_containers/pause3.0 "/pause"               3 hours ago Up 3 ho       k8s_PO..
d44ea81b29f1 gcr.io/google_containers/pause3.0 "/pause"               3 hours ago Up 3 ho       k8s_PO..
3b26e522321b d166ffa9201a                      "/opt/kube-addons.sh"  3 hours ago Up 3 ho       k8s_ku..
5a0a0833416d gcr.io/google_containers/pause3.0 "/pause"               3 hours ago Up 3 ho       k8s_PO..

$ exit

# to stop minikube, do like follows
[root@lab ~]# minikube stop 
Stopping local Kubernetes cluster...
Machine stopped.

# to remove minikube, do like follows
[root@lab ~]# minikube delete 
Deleting local Kubernetes cluster...
Machine deleted.

[root@lab ~]# virsh list --all 
 Id    Name                           State
----------------------------------------------------
```

## 1.3 创建删除Pods

```Shell
# run 2 [test-nginx] pods
[root@lab ~]# kubectl run test-nginx --image=nginx --replicas=2 --port=80 
deployment.apps "test-nginx" created

[root@lab ~]# kubectl get pods 
NAME                         READY     STATUS    RESTARTS   AGE
test-nginx-c8b797d7d-mzf9h   1/1       Running   0          25s
test-nginx-c8b797d7d-zh78r   1/1       Running   0          25s

# show environment variable for [test-nginx] pod
[root@lab ~]# kubectl exec test-nginx-c8b797d7d-mzf9h env 
PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
HOSTNAME=test-nginx-c8b797d7d-mzf9h
KUBERNETES_SERVICE_PORT_HTTPS=443
KUBERNETES_PORT=tcp://10.96.0.1:443
KUBERNETES_PORT_443_TCP=tcp://10.96.0.1:443
KUBERNETES_PORT_443_TCP_PROTO=tcp
KUBERNETES_PORT_443_TCP_PORT=443
KUBERNETES_PORT_443_TCP_ADDR=10.96.0.1
KUBERNETES_SERVICE_HOST=10.96.0.1
KUBERNETES_SERVICE_PORT=443
NGINX_VERSION=1.13.11-1~stretch
NJS_VERSION=1.13.11.0.2.0-1~stretch
HOME=/root

# shell access to [test-nginx] pod
[root@lab ~]# kubectl exec -it test-nginx-c8b797d7d-mzf9h bash
root@test-nginx-c8b797d7d-mzf9h:/# hostname 
test-nginx-c8b797d7d-mzf9h
root@test-nginx-c8b797d7d-mzf9h:/# curl localhost 
<!DOCTYPE html>
<html>
<head>
<title>Welcome to nginx!</title>
.....
.....
<p><em>Thank you for using nginx.</em></p>
</body>
</html>

root@test-nginx-c8b797d7d-mzf9h:/# exit

# show logs of [test-nginx] pod
[root@lab ~]# kubectl logs test-nginx-c8b797d7d-mzf9h 
127.0.0.1 - - [09/Apr/2018:07:51:22 +0000] "GET / HTTP/1.1" 200 612 "-" "curl/7.52.1" "-"

# scale out pods
[root@lab ~]# kubectl scale deployment test-nginx --replicas=3 
deployment.extensions "test-nginx" scaled

[root@lab ~]# kubectl get pods 
NAME                         READY     STATUS    RESTARTS   AGE
test-nginx-c8b797d7d-kdgmk   1/1       Running   0          7s
test-nginx-c8b797d7d-mzf9h   1/1       Running   0          4m
test-nginx-c8b797d7d-zh78r   1/1       Running   0          4m

# set service
[root@lab ~]# kubectl expose deployment test-nginx --type="NodePort" --port 80 
service "test-nginx" exposed

[root@lab ~]# kubectl get services test-nginx 
NAME         TYPE       CLUSTER-IP      EXTERNAL-IP   PORT(S)        AGE
test-nginx   NodePort   10.99.186.171   <none>        80:31495/TCP   11s

[root@lab ~]# minikube service test-nginx --url 
http://192.168.39.110:31495

[root@lab ~]# curl http://192.168.39.110:31495 
<!DOCTYPE html>
<html>
<head>
<title>Welcome to nginx!</title>
.....
.....
<p><em>Thank you for using nginx.</em></p>
</body>
</html>

# delete service
[root@lab ~]# kubectl delete services test-nginx 
service "test-nginx" deleted

# delete pods
[root@lab ~]# kubectl delete deployment test-nginx 
deployment.extensions "test-nginx" deleted
```

# 2. 使用Kubeadm配置多节点Kubernetes集群

实验环境：
```
 -----------+---------------------------+--------------------------+------------
            |                           |                          |
        eth0|10.0.0.30              eth0|10.0.0.51             eth0|10.0.0.52
 +----------+-----------+   +-----------+----------+   +-----------+----------+
 |   [ lab.centos.org]  |   | [ node01.centos.org] |   | [ node02.centos.org] |
 |      Master Node     |   |      Worker Node     |   |      Worker Node     |
 +----------------------+   +----------------------+   +----------------------+
```

前提条件：

1. 在所有节点上安装运行Docker服务
2. 禁用SELinux（因为kubelet不支持）
3. 禁用Firewalld防火墙服务
4. 在所有节点上禁用Swap交换空间及配置系统参数
```Shell
[root@lab ~]# swapoff -a 

[root@lab ~]# vi /etc/fstab

# disable swap line
#/dev/mapper/cl-swap swap swap defaults 0 0

[root@lab ~]# cat <<EOF > /etc/sysctl.d/k8s.conf
net.bridge.bridge-nf-call-ip6tables = 1
net.bridge.bridge-nf-call-iptables = 1
EOF
[root@lab ~]# sysctl --system 
```
## 2.1 在所有节点上安装Kubeadm

```Shell
[root@lab ~]# cat <<'EOF' > /etc/yum.repos.d/kubernetes.repo
[kubernetes]
name=Kubernetes
baseurl=https://packages.cloud.google.com/yum/repos/kubernetes-el7-$basearch
enabled=1
gpgcheck=1
repo_gpgcheck=1
gpgkey=https://packages.cloud.google.com/yum/doc/yum-key.gpg https://packages.cloud.google.com/yum/doc/rpm-package-key.gpg
EOF

[root@lab ~]# yum -y install kubeadm kubelet kubectl

# only enabling, do not run yet
[root@lab ~]# systemctl enable kubelet 
```

## 2.2 配置集群

--apiserver-advertise-address 选项：指定Kubernetes API server监听地址。

--pod-network-cidr 选项：指定Pod网络范围。

Kubernetes支持插件是Pod网络，在下面例子中我们选择使用Flannel。

**1. 在主节点上初始化配置**

```Shell
[root@lab ~]# kubeadm init --apiserver-advertise-address=10.0.0.30 --pod-network-cidr=10.244.0.0/16 
[init] Using Kubernetes version: v1.10.0
[init] Using Authorization modes: [Node RBAC]
[preflight] Running pre-flight checks.
[WARNING Firewalld]: firewalld is active, please ensure ports [6443 10250] are open or your cluster may not function correctly
[preflight] Starting the kubelet service
[certificates] Generated ca certificate and key.
[certificates] Generated apiserver certificate and key.
[certificates] apiserver serving cert is signed for DNS names [lab.centos.org kubernetes kubernetes.default kubernetes.default.svc kubernetes.default.svc.cluster.local] and IPs [10.96.0.1 10.0.0.30]
.....
.....
[bootstraptoken] Configured RBAC rules to allow the csrapprover controller automatically approve CSRs from a Node Bootstrap Token
[bootstraptoken] Configured RBAC rules to allow certificate rotation for all node client certificates in the cluster
[bootstraptoken] Creating the "cluster-info" ConfigMap in the "kube-public" namespace
[addons] Applied essential addon: kube-dns
[addons] Applied essential addon: kube-proxy

Your Kubernetes master has initialized successfully!

To start using your cluster, you need to run the following as a regular user:

  mkdir -p $HOME/.kube
  sudo cp -i /etc/kubernetes/admin.conf $HOME/.kube/config
  sudo chown $(id -u):$(id -g) $HOME/.kube/config

You should now deploy a pod network to the cluster.
Run "kubectl apply -f [podnetwork].yaml" with one of the options listed at:
  https://kubernetes.io/docs/concepts/cluster-administration/addons/

You can now join any number of machines by running the following on each node
as root:

  # the command below is necessary to run on Worker Node when he joins to the cluster, so remember it
  kubeadm join 10.0.0.30:6443 --token ivcdtn.r9qt329oe49nb3b7 --discovery-token-ca-cert-hash sha256:2a2bdff5648e6f17bbc60889e8b47656795f2cb2ea959c8dc10b5dcb09d48be5

# set cluster admin user
# if you set common user as cluster admin, login with it and run [sudo cp/chown ***]
[root@lab ~]# mkdir -p $HOME/.kube 
[root@lab ~]# cp -i /etc/kubernetes/admin.conf $HOME/.kube/config 
[root@lab ~]# chown $(id -u):$(id -g) $HOME/.kube/config
```

**2. Pod网络配置**

```Shell
[root@lab ~]# kubectl apply -f https://raw.githubusercontent.com/coreos/flannel/master/Documentation/kube-flannel.yml 
clusterrole.rbac.authorization.k8s.io "flannel" created
clusterrolebinding.rbac.authorization.k8s.io "flannel" created
serviceaccount "flannel" created
configmap "kube-flannel-cfg" created
daemonset.extensions "kube-flannel-ds" created

# show state (OK if STATUS = Ready)
[root@lab ~]# kubectl get nodes 
NAME            STATUS    ROLES     AGE       VERSION
lab.centos.org   Ready     master    18m       v1.10.0

# show state (OK if all are Running)
[root@lab ~]# kubectl get pods --all-namespaces 
NAMESPACE     NAME                                    READY     STATUS    RESTARTS   AGE
kube-system   etcd-lab.centos.org                     1/1       Running   0          17m
kube-system   kube-apiserver-lab.centos.org           1/1       Running   0          17m
kube-system   kube-controller-manager-lab.centos.org  1/1       Running   0          17m
kube-system   kube-dns-86f4d74b45-rtrgd               3/3       Running   0          18m
kube-system   kube-flannel-ds-8lhqq                   1/1       Running   0          1m
kube-system   kube-proxy-cqmfz                        1/1       Running   0          18m
kube-system   kube-scheduler-lab.centos.org           1/1       Running   0          17m
```

## 2.3 将工作节点加入到集群

**1. 加入到已初始化的Kubernetes集群**

```Shell
[root@node01 ~]# kubeadm join 10.0.0.30:6443 --token ivcdtn.r9qt329oe49nb3b7 --discovery-token-ca-cert-hash sha256:2a2bdff5648e6f17bbc60889e8b47656795f2cb2ea959c8dc10b5dcb09d48be5 
[preflight] Running pre-flight checks.
        [WARNING FileExisting-crictl]: crictl not found in system path
Suggestion: go get github.com/kubernetes-incubator/cri-tools/cmd/crictl
[preflight] Starting the kubelet service
[discovery] Trying to connect to API Server "10.0.0.30:6443"
[discovery] Created cluster-info discovery client, requesting info from "https://10.0.0.30:6443"
[discovery] Requesting info from "https://10.0.0.30:6443" again to validate TLS against the pinned public key
[discovery] Cluster info signature and contents are valid and TLS certificate validates against pinned roots, will use API Server "10.0.0.30:6443"
[discovery] Successfully established connection with API Server "10.0.0.30:6443"

This node has joined the cluster:
* Certificate signing request was sent to master and a response
  was received.
* The Kubelet was informed of the new secure connection details.

Run 'kubectl get nodes' on the master to see this node join the cluster.

# OK if shown [Successfully established connection ***]
```

**2. 在主节点上确认各节点状态**

```Shell
[root@lab ~]# kubectl get nodes 
NAME               STATUS    ROLES     AGE       VERSION
lab.centos.org      Ready     master    1h        v1.10.0
node01.centos.org   Ready     <none>    4m        v1.10.0
node02.centos.org   Ready     <none>    2m        v1.10.0
```

**3. 部署Pod测试**

```Shell
[root@lab ~]# kubectl run test-nginx --image=nginx --replicas=2 --port=80 
deployment.apps "test-nginx" created

[root@lab ~]# kubectl get pods -o wide 
NAME                         READY     STATUS    RESTARTS   AGE       IP           NODE
test-nginx-959dbd6b6-dkplf   1/1       Running   0          14m       10.244.1.2   node01.centos.org
test-nginx-959dbd6b6-qhz55   1/1       Running   0          14m       10.244.2.2   node02.centos.org

[root@lab ~]# kubectl expose deployment test-nginx 
service "test-nginx" exposed

[root@lab ~]# kubectl describe service test-nginx 
Name:              test-nginx
Namespace:         default
Labels:            run=test-nginx
Annotations:       <none>
Selector:          run=test-nginx
Type:              ClusterIP
IP:                10.102.121.104
Port:              <unset>  80/TCP
TargetPort:        80/TCP
Endpoints:         10.244.1.2:80,10.244.2.2:80
Session Affinity:  None
Events:            <none>

[root@lab ~]# curl 10.102.121.104 
<!DOCTYPE html>
<html>
<head>
<title>Welcome to nginx!</title>
.....
.....
<p><em>Thank you for using nginx.</em></p>
</body>
</html>
```

## 2.4 使用持久存储

实验环境：
```
 -----------+---------------------------+--------------------------+------------
            |                           |                          |
        eth0|10.0.0.30              eth0|10.0.0.51             eth0|10.0.0.52
 +----------+-----------+   +-----------+----------+   +-----------+----------+
 |   [ lab.centos.org]  |   | [ node01.centos.org] |   | [ node02.centos.org] |
 |      Master Node     |   |      Worker Node     |   |      Worker Node     |
 |      NFS Server      |   |                      |   |                      |
 +----------------------+   +----------------------+   +----------------------+
 ```
 
**1. 配置NFS共享存储[/var/lib/nfs-share]**

> 参见其NFS配置章节
    
**2. 在主节点上定义PV(Persistent Volume)对象和PVC(Persistent Volume Claim)对象**

```Shell
# create PV definition
[root@lab ~]# vi nfs-pv.yml

apiVersion: v1
kind: PersistentVolume
metadata:
  # any PV name
  name: nfs-pv
spec:
  capacity:
    # storage size
    storage: 10Gi
  accessModes:
    # ReadWriteMany(RW from multi nodes), ReadWriteOnce(RW from a node), ReadOnlyMany(R from multi nodes)
    - ReadWriteMany
  persistentVolumeReclaimPolicy:
    # retain even if pods terminate
    Retain
  nfs:
    # NFS server's definition
    path: /var/lib/nfs-share
    server: 10.0.0.30
    readOnly: false

[root@lab ~]# kubectl create -f nfs-pv.yml 
persistentvolume "nfs-pv" created

[root@lab ~]# kubectl get pv 
NAME      CAPACITY   ACCESS MODES   RECLAIM POLICY   STATUS    CLAIM             STORAGECLASS   REASON    AGE
nfs-pv    10Gi       RWX            Retain           Bound     default/nfs-pvc                            2d

# create PVC definition
[root@lab ~]# vi nfs-pvc.yml

apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  # any PVC name
  name: nfs-pvc
spec:
  accessModes:
  # ReadWriteMany(RW from multi nodes), ReadWriteOnce(RW from a node), ReadOnlyMany(R from multi nodes)
  - ReadWriteMany
  resources:
     requests:
       # storage size to use
       storage: 1Gi

[root@lab ~]# kubectl create -f nfs-pvc.yml 
persistentvolumeclaim "nfs-pvc" created

[root@lab ~]# kubectl get pvc 
NAME      STATUS    VOLUME    CAPACITY   ACCESS MODES   STORAGECLASS   AGE
nfs-pvc   Bound     nfs-pv    10Gi       RWX                           2d
```

**3. 在PVC上面创建Pod**

```Shell
[root@lab ~]# vi nginx-nfs.yml

apiVersion: v1
kind: Pod
metadata:
  # any Pod name
  name: nginx-nfs
  labels:
    name: nginx-nfs
spec:
  containers:
    - name: nginx-nfs
      image: fedora/nginx
      ports:
        - name: web
          containerPort: 80
      volumeMounts:
        - name: nfs-share
          # mount point in container
          mountPath: /usr/share/nginx/html
  volumes:
    - name: nfs-share
      persistentVolumeClaim:
        # PVC name you created
        claimName: nfs-pvc

[root@lab ~]# kubectl create -f nginx-nfs.yml 
pod "nginx-nfs" created

# create a test file under the NFS shared directory
[root@lab ~]# echo 'NFS Persistent Storage Test' > /var/lib/nfs-share/index.html
[root@lab ~]# kubectl get pods -o wide 
NAME        READY     STATUS    RESTARTS   AGE       IP            NODE
nginx-nfs   1/1       Running   0          3m        10.244.1.10   node01.srv.world

# verify accessing
[root@lab ~]# curl 10.244.1.10 
NFS Persistent Storage Test
```

## 2.5 使用私有仓库

**1. 配置秘钥**

```Shell
# login to the Registry once
[root@lab ~]# docker login lab.centos.org:5000 
Username: admin
Password:
Login Succeeded

# then following file is generated
[root@lab ~]# ll ~/.docker/config.json 
-rw-------. 1 root root 82 Apr 15 12:53 /root/.docker/config.json

# BASE64 encode of the file
[root@lab ~]# cat ~/.docker/config.json | base64 
ewoJImF1dGhzIjogewoJCSJkbHAuc3J2LndvcmxkOjUwMDAiOiB7CgkJCSJ...

[root@lab ~]# vi regcred.yml
# create new
# specify contents of BASE64 encoding above with one line for [.dockerconfigjson] section
apiVersion: v1
kind: Secret
data:
  .dockerconfigjson: ewoJImF1dGhzIjogewoJCSJkbHAuc3J2LndvcmxkOjUwMDAiOiB7CgkJCSJ...
metadata:
  name: regcred
type: kubernetes.io/dockerconfigjson

[root@lab ~]# kubectl create -f regcred.yml 
secret "regcred" created
[root@lab ~]# kubectl get secrets 
NAME                  TYPE                                  DATA      AGE
default-token-8gcdr   kubernetes.io/service-account-token   3         3d
regcred               kubernetes.io/dockerconfigjson        1         5m
```

**2. 使用私有仓库部署Pod**

```Shell
[root@lab ~]# docker images lab.centos.org:5000/nginx 
REPOSITORY                 TAG                 IMAGE ID            CREATED             SIZE
lab.centos.org:5000/nginx  latest              b175e7467d66        6 days ago          109 MB

[root@lab ~]# vi private-nginx.yml
apiVersion: v1
kind: Pod
metadata:
  name: private-nginx
spec:
  containers:
  - name: private-nginx
    # image on Private Registry
    image: lab.centos.org:5000/nginx
  imagePullSecrets:
  # Secret name you added
  - name: regcred

[root@lab ~]# kubectl create -f private-nginx.yml 
pod "private-nginx" created

[root@lab ~]# kubectl get pods 
NAME            READY     STATUS    RESTARTS   AGE
private-nginx   1/1       Running   0          41s

[root@lab ~]# kubectl describe pods private-nginx 
Name:         private-nginx
Namespace:    default
Node:         node01.centos.org/10.0.0.51
Start Time:   Tue, 15 Apr 2018 13:33:21 +0900
Labels:       <none>
Annotations:  <none>
Status:       Running
IP:           10.244.1.11
Containers:
  private-nginx:
    Container ID:   docker://0e805ca076040e6a510b3a0520cb6c4593431484b32b4b6fb9bec2317026d076
    Image:          lab.centos.org:5000/nginx
    Image ID:       docker-pullable://lab.centos.org:5000/nginx@sha256:d903fe3076f89ad76afe1cb...
.....
.....
Events:
  Type   Reason       Age  From                        Message
  ----   ------       ---- ----                        -------
  Normal Scheduled    1m   default-scheduler           Successfully assigned private-nginx to node01..
  Normal Successful.. 1m   kubelet, node01.centos.org  MountVolume.SetUp succeeded for volume "defau..
  Normal Pulling      1m   kubelet, node01.centos.org  pulling image "lab.centos.org:5000/nginx"
  Normal Pulled       1m   kubelet, node01.centos.org  Successfully pulled image "lab.centos.org:5000/nginx"
  Normal Created      1m   kubelet, node01.centos.org  Created container
  Normal Started      1m   kubelet, node01.centos.org  Started container
```
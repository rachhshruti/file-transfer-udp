Vagrant.configure("2") do |config|
  config.vm.define "server" do |server|
    server.vm.box = "ubuntu/trusty64"
    server.vm.hostname = 'server'
    server.vm.box_url = "ubuntu/trusty64"

    server.vm.network :private_network, ip: "10.0.0.10"
    server.vm.network :forwarded_port, guest: 1030, host: 1254, protocol: "udp"

    server.vm.provider :virtualbox do |v|
      v.customize ["modifyvm", :id, "--natdnshostresolver1", "on"]
      v.customize ["modifyvm", :id, "--memory", 1024]
      v.customize ["modifyvm", :id, "--name", "server"]
    end
  end

  config.vm.define "client" do |client|
    client.vm.box = "ubuntu/trusty64"
    client.vm.hostname = 'client'
    client.vm.box_url = "ubuntu/trusty64"

    client.vm.network :private_network, ip: "10.0.0.11"
    client.vm.network :forwarded_port, guest: 1030, host: 1255, protocol: "udp"

    client.vm.provider :virtualbox do |v|
      v.customize ["modifyvm", :id, "--natdnshostresolver1", "on"]
      v.customize ["modifyvm", :id, "--memory", 1024]
      v.customize ["modifyvm", :id, "--name", "client"]
    end
  end
  
  config.vm.provision "shell", inline: <<-SHELL
      apt-get update
      apt-get install -y g++
    SHELL
end
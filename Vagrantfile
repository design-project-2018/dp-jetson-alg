Vagrant.configure(2) do |config|
  config.vm.box = "arnemertz/Xubuntu16.04"
  config.vm.provider "virtualbox" do |vb|
    vb.gui = true
	vb.name = "Xubuntu1604_DevBox"
	vb.customize ["modifyvm", :id, "--memory", "4096"]
	vb.customize ["modifyvm", :id, "--cpus", "3"]
	vb.customize ["modifyvm", :id, "--ioapic", "on"]
	vb.customize ["modifyvm", :id, "--hwvirtex", "on"]
	vb.customize ["modifyvm", :id, "--accelerate3d", "on"]
	vb.customize ['modifyvm', :id, '--clipboard', 'bidirectional']
  end
  config.vm.provision :shell, :path => File.join( "provision", "provision.sh" )
end
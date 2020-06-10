#! /bin/bash


### libary ###

function add_comma_every_eight
{
        echo " $1 " | sed -r ':L;s=\b([0-9]+)([0-9]{8})\b=\1,\2=g;t L'
}

function int2hex
{
        CHUNKS=$(( $1/64 ))
        COREID=$1
        HEX=""
        for (( CHUNK=0; CHUNK<${CHUNKS} ; CHUNK++ ))
        do
                HEX=$HEX"0000000000000000"
                COREID=$((COREID-64))
        done
        printf "%x$HEX" $(echo $((2**$COREID)) )
}


function core_to_affinity
{
        echo $( add_comma_every_eight $( int2hex $1) )
}

function get_irq_list
{
        interface=$1
        infiniband_device_irqs_path="/sys/class/infiniband/$interface/device/msi_irqs"
        net_device_irqs_path="/sys/class/net/$interface/device/msi_irqs"
        interface_in_proc_interrupts=$( cat /proc/interrupts | egrep "$interface[^0-9,a-z,A-Z]" | awk '{print $1}' | sed 's/://' )
        if [ -d $infiniband_device_irqs_path ]; then
                irq_list=$( /bin/ls $infiniband_device_irqs_path )
        elif [ "$interface_in_proc_interrupts" != "" ]; then
                irq_list=$interface_in_proc_interrupts
        elif [ -d $net_device_irqs_path ]; then
                irq_list=$( /bin/ls $net_device_irqs_path )
        else
                echo "Error - interface or device \"$interface\" does not exist" 1>&2
                exit 1
        fi
        echo $irq_list
}

function show_irq_affinity
{
        irq_num=$1
        smp_affinity_path="/proc/irq/$irq_num/smp_affinity"
        if [ -f $smp_affinity_path ]; then
                echo -n "$irq_num: "
                cat $smp_affinity_path
        fi
}

function show_irq_affinity_hints
{
        irq_num=$1
        affinity_hint_path="/proc/irq/$irq_num/affinity_hint"
        if [ -f $affinity_hint_path ]; then
                echo -n "$irq_num: "
                cat $affinity_hint_path
        fi
}

function set_irq_affinity
{
        irq_num=$1
        affinity_mask=$2
        smp_affinity_path="/proc/irq/$irq_num/smp_affinity"
        if [ -f $smp_affinity_path ]; then
                echo $affinity_mask > $smp_affinity_path
        fi
}

function is_affinity_hint_set
{
        irq_num=$1
        hint_not_set=0
        affinity_hint_path="/proc/irq/$irq_num/affinity_hint"
        if [ -f $affinity_hint_path ]; then
                TOTAL_CHAR=$( wc -c < $affinity_hint_path  )
                NUM_OF_COMMAS=$( grep -o "," $affinity_hint_path | wc -l )
                NUM_OF_ZERO=$( grep -o "0" $affinity_hint_path | wc -l )
                NUM_OF_F=$( grep -i -o "f" $affinity_hint_path | wc -l )
                if [[ $((TOTAL_CHAR-1-NUM_OF_COMMAS)) -eq $NUM_OF_ZERO || $((TOTAL_CHAR-1-NUM_OF_COMMAS)) -eq $NUM_OF_F ]]; then
                        hint_not_set=1
                fi
        else
                hint_not_set=1
        fi
        return $hint_not_set
}


if [ -z $1 ]; then
	echo "usage: $0 <NIC interface> [2nd NIC interface]"
	exit 1
fi

source common_irq_affinity.sh

CORES=$((`cat /proc/cpuinfo | grep processor | tail -1 | awk '{print $3}'`+1))
hop=1
INT1=$1
INT2=$2
CNT1=0
CNT2=0

if [ -z $INT2 ]; then
	limit_1=$CORES
	echo "---------------------------------------"
	echo "Optimizing IRQs for Single port traffic"
	echo "---------------------------------------"
else
	echo "-------------------------------------"
	echo "Optimizing IRQs for Dual port traffic"
	echo "-------------------------------------"
	limit_1=$((CORES/2))
	limit_2=$CORES
	IRQS_2=$( get_irq_list $INT2 )
	if [ -z "$IRQS_2" ] ; then
		echo No IRQs found for $INT2.
		exit 1
	fi
fi

IRQS_1=$( get_irq_list $INT1 )

if [ -z "$IRQS_1" ] ; then
	echo No IRQs found for $INT1.
else
	echo Discovered irqs for $INT1: $IRQS_1
	core_id=0
	for IRQ in $IRQS_1
	do
		if is_affinity_hint_set $IRQ ; then
			if [ $CNT1 -lt 8 ] ; then
				affinity=$(cat /proc/irq/$[$IRQ+8]/affinity_hint)
			elif [ $CNT1 -lt 16 ] ; then
				affinity=$(cat /proc/irq/$[$IRQ-8]/affinity_hint)
			elif [ $CNT1 -lt 24 ] ; then
				affinity=$(cat /proc/irq/$[$IRQ+8]/affinity_hint)
			elif [ $CNT1 -lt 32 ] ; then
				affinity=$(cat /proc/irq/$[$IRQ-8]/affinity_hint)
			elif [ $CNT1 -lt 40 ] ; then
				affinity=$(cat /proc/irq/$[$IRQ+8]/affinity_hint)
			elif [ $CNT1 -lt 48 ] ; then
				affinity=$(cat /proc/irq/$[$IRQ-8]/affinity_hint)
			elif [ $CNT1 -lt 56 ] ; then
				affinity=$(cat /proc/irq/$[$IRQ+8]/affinity_hint)
			elif [ $CNT1 -lt 64 ] ; then
				affinity=$(cat /proc/irq/$[$IRQ-8]/affinity_hint)
			fi
			CNT1=$[$CNT1+1];
			if [ $CNT2 -gt 63 ] ; then
				CNT1=0
			fi
			set_irq_affinity $IRQ $affinity
			echo Assign irq $IRQ to its affinity_hint $affinity
		else
			if [ $CNT1 -lt 8 ] ; then
				affinity=$(cat /proc/irq/$[$IRQ+8]/affinity_hint)
			elif [ $CNT1 -lt 16 ] ; then
				affinity=$(cat /proc/irq/$[$IRQ-8]/affinity_hint)
			elif [ $CNT1 -lt 24 ] ; then
				affinity=$(cat /proc/irq/$[$IRQ+8]/affinity_hint)
			elif [ $CNT1 -lt 32 ] ; then
				affinity=$(cat /proc/irq/$[$IRQ-8]/affinity_hint)
			elif [ $CNT1 -lt 40 ] ; then
				affinity=$(cat /proc/irq/$[$IRQ+8]/affinity_hint)
			elif [ $CNT1 -lt 48 ] ; then
				affinity=$(cat /proc/irq/$[$IRQ-8]/affinity_hint)
			elif [ $CNT1 -lt 56 ] ; then
				affinity=$(cat /proc/irq/$[$IRQ+8]/affinity_hint)
			elif [ $CNT1 -lt 64 ] ; then
				affinity=$(cat /proc/irq/$[$IRQ-8]/affinity_hint)
			fi
			CNT1=$[$CNT1+1];
			if [ $CNT2 -gt 63 ] ; then
				CNT1=0
			fi
			echo Assign irq $IRQ core_id $core_id
			affinity=$( core_to_affinity $core_id )
			set_irq_affinity $IRQ $affinity
			core_id=$(( core_id + $hop ))
			if [ $core_id -ge $limit_1 ] ; then core_id=0; fi
		fi
	done
fi

echo 

if [ "$INT2" != "" ]; then
	IRQS_2=$( get_irq_list $INT2 )
	if [ -z "$IRQS_2" ]; then
		echo No IRQs found for $INT2.
		exit 1
	fi

	echo Discovered irqs for $INT2: $IRQS_2
	core_id=$limit_1
	for IRQ in $IRQS_2
	do
		if is_affinity_hint_set $IRQ ; then
			if [ $CNT1 -lt 8 ] ; then
				affinity=$(cat /proc/irq/$[$IRQ+8]/affinity_hint)
			elif [ $CNT1 -lt 16 ] ; then
				affinity=$(cat /proc/irq/$[$IRQ-8]/affinity_hint)
			elif [ $CNT1 -lt 24 ] ; then
				affinity=$(cat /proc/irq/$[$IRQ+8]/affinity_hint)
			elif [ $CNT1 -lt 32 ] ; then
				affinity=$(cat /proc/irq/$[$IRQ-8]/affinity_hint)
			elif [ $CNT1 -lt 40 ] ; then
				affinity=$(cat /proc/irq/$[$IRQ+8]/affinity_hint)
			elif [ $CNT1 -lt 48 ] ; then
				affinity=$(cat /proc/irq/$[$IRQ-8]/affinity_hint)
			elif [ $CNT1 -lt 56 ] ; then
				affinity=$(cat /proc/irq/$[$IRQ+8]/affinity_hint)
			elif [ $CNT1 -lt 64 ] ; then
				affinity=$(cat /proc/irq/$[$IRQ-8]/affinity_hint)
			fi
			CNT2=$[$CNT2+1];
			if [ $CNT2 -gt 63 ] ; then
				CNT2=0
			fi
			affinity=$(cat /proc/irq/$IRQ/affinity_hint)
			set_irq_affinity $IRQ $affinity
			echo Assign irq $IRQ to its affinity_hint $affinity
		else
			if [ $CNT1 -lt 8 ] ; then
				affinity=$(cat /proc/irq/$[$IRQ+8]/affinity_hint)
			elif [ $CNT1 -lt 16 ] ; then
				affinity=$(cat /proc/irq/$[$IRQ-8]/affinity_hint)
			elif [ $CNT1 -lt 24 ] ; then
				affinity=$(cat /proc/irq/$[$IRQ+8]/affinity_hint)
			elif [ $CNT1 -lt 32 ] ; then
				affinity=$(cat /proc/irq/$[$IRQ-8]/affinity_hint)
			elif [ $CNT1 -lt 40 ] ; then
				affinity=$(cat /proc/irq/$[$IRQ+8]/affinity_hint)
			elif [ $CNT1 -lt 48 ] ; then
				affinity=$(cat /proc/irq/$[$IRQ-8]/affinity_hint)
			elif [ $CNT1 -lt 56 ] ; then
				affinity=$(cat /proc/irq/$[$IRQ+8]/affinity_hint)
			elif [ $CNT1 -lt 64 ] ; then
				affinity=$(cat /proc/irq/$[$IRQ-8]/affinity_hint)
			fi
			CNT2=$[$CNT2+1];
			if [ $CNT2 -gt 63 ] ; then
				CNT2=0
			fi
			echo Assign irq $IRQ core_id $core_id
			affinity=$( core_to_affinity $core_id )
			set_irq_affinity $IRQ $affinity
			core_id=$(( core_id + $hop ))
			if [ $core_id -ge $limit_2 ] ; then core_id=$limit_1; fi
		fi
	done
fi
echo
echo done.
